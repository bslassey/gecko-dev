/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TreeWalker.h"

#include "Accessible.h"
#include "AccIterator.h"
#include "nsAccessibilityService.h"
#include "DocAccessible.h"

#include "mozilla/dom/ChildIterator.h"
#include "mozilla/dom/Element.h"

using namespace mozilla;
using namespace mozilla::a11y;

////////////////////////////////////////////////////////////////////////////////
// TreeWalker
////////////////////////////////////////////////////////////////////////////////

TreeWalker::
  TreeWalker(Accessible* aContext) :
  mDoc(aContext->Document()), mContext(aContext), mAnchorNode(nullptr),
  mARIAOwnsIdx(0),
  mChildFilter(nsIContent::eSkipPlaceholderContent), mFlags(0),
  mPhase(eAtStart)
{
  mChildFilter |= mContext->NoXBLKids() ?
    nsIContent::eAllButXBL : nsIContent::eAllChildren;

  mAnchorNode = mContext->IsDoc() ?
    mDoc->DocumentNode()->GetRootElement() : mContext->GetContent();

  MOZ_COUNT_CTOR(TreeWalker);
}

TreeWalker::
  TreeWalker(Accessible* aContext, nsIContent* aAnchorNode, uint32_t aFlags) :
  mDoc(aContext->Document()), mContext(aContext), mAnchorNode(aAnchorNode),
  mARIAOwnsIdx(0),
  mChildFilter(nsIContent::eSkipPlaceholderContent), mFlags(aFlags),
  mPhase(eAtStart)
{
  MOZ_ASSERT(aAnchorNode, "No anchor node for the accessible tree walker");
  MOZ_ASSERT(mDoc->GetAccessibleOrContainer(aAnchorNode) == mContext,
             "Unexpected anchor node was given");

  mChildFilter |= mContext->NoXBLKids() ?
    nsIContent::eAllButXBL : nsIContent::eAllChildren;

  MOZ_COUNT_CTOR(TreeWalker);
}

TreeWalker::~TreeWalker()
{
  MOZ_COUNT_DTOR(TreeWalker);
}

bool
TreeWalker::Seek(nsIContent* aChildNode)
{
  MOZ_ASSERT(aChildNode, "Child cannot be null");

  mPhase = eAtStart;
  mStateStack.Clear();
  mARIAOwnsIdx = 0;

  nsIContent* childNode = nullptr;
  nsINode* parentNode = aChildNode;
  do {
    childNode = parentNode->AsContent();
    parentNode = childNode->HasFlag(NODE_MAY_BE_IN_BINDING_MNGR) &&
      (mChildFilter & nsIContent::eAllButXBL) ?
      childNode->GetParentNode() : childNode->GetFlattenedTreeParent();

    if (!parentNode || !parentNode->IsElement()) {
      return false;
    }

    // If ARIA owned child.
    Accessible* child = mDoc->GetAccessible(childNode);
    if (child && child->IsRelocated()) {
      if (child->Parent() != mContext) {
        return false;
      }

      Accessible* ownedChild = nullptr;
      while ((ownedChild = mDoc->ARIAOwnedAt(mContext, mARIAOwnsIdx++)) &&
             ownedChild != child);

      MOZ_ASSERT(ownedChild, "A child has to be in ARIA owned elements");
      mPhase = eAtARIAOwns;
      return true;
    }

    // Look in DOM.
    dom::AllChildrenIterator* iter = PrependState(parentNode->AsElement(), true);
    if (!iter->Seek(childNode)) {
      return false;
    }

    if (parentNode == mAnchorNode) {
      mPhase = eAtDOM;
      return true;
    }
  } while (true);

  return false;
}

Accessible*
TreeWalker::Next()
{
  if (mStateStack.IsEmpty()) {
    if (mPhase == eAtEnd) {
      return nullptr;
    }

    if (mPhase == eAtDOM || mPhase == eAtARIAOwns) {
      mPhase = eAtARIAOwns;
      Accessible* child = mDoc->ARIAOwnedAt(mContext, mARIAOwnsIdx);
      if (child) {
        mARIAOwnsIdx++;
        return child;
      }
      mPhase = eAtEnd;
      return nullptr;
    }

    if (!mAnchorNode) {
      mPhase = eAtEnd;
      return nullptr;
    }

    mPhase = eAtDOM;
    PushState(mAnchorNode, true);
  }

  dom::AllChildrenIterator* top = &mStateStack[mStateStack.Length() - 1];
  while (top) {
    while (nsIContent* childNode = top->GetNextChild()) {
      bool skipSubtree = false;
      Accessible* child = nullptr;
      if (mFlags & eWalkCache) {
        child = mDoc->GetAccessible(childNode);
      }
      else if (mContext->IsAcceptableChild(childNode)) {
        child = GetAccService()->
          GetOrCreateAccessible(childNode, mContext, &skipSubtree);
      }

      // Ignore the accessible and its subtree if it was repositioned by means
      // of aria-owns.
      if (child) {
        if (child->IsRelocated()) {
          continue;
        }
        return child;
      }

      // Walk down into subtree to find accessibles.
      if (!skipSubtree && childNode->IsElement()) {
        top = PushState(childNode, true);
      }
    }
    top = PopState();
  }

  // If we traversed the whole subtree of the anchor node. Move to next node
  // relative anchor node within the context subtree if possible.
  if (mFlags != eWalkContextTree)
    return Next();

  nsINode* contextNode = mContext->GetNode();
  while (mAnchorNode != contextNode) {
    nsINode* parentNode = mAnchorNode->GetFlattenedTreeParent();
    if (!parentNode || !parentNode->IsElement())
      return nullptr;

    nsIContent* parent = parentNode->AsElement();
    top = PushState(parent, true);
    if (top->Seek(mAnchorNode)) {
      mAnchorNode = parent;
      return Next();
    }

    // XXX We really should never get here, it means we're trying to find an
    // accessible for a dom node where iterating over its parent's children
    // doesn't return it. However this sometimes happens when we're asked for
    // the nearest accessible to place holder content which we ignore.
    mAnchorNode = parent;
  }

  return Next();
}

Accessible*
TreeWalker::Prev()
{
  if (mStateStack.IsEmpty()) {
    if (mPhase == eAtStart || mPhase == eAtDOM) {
      mPhase = eAtStart;
      return nullptr;
    }

    if (mPhase == eAtEnd) {
      mARIAOwnsIdx = mDoc->ARIAOwnedCount(mContext);
      mPhase = eAtARIAOwns;
    }

    if (mPhase == eAtARIAOwns) {
      if (mARIAOwnsIdx > 0) {
        return mDoc->ARIAOwnedAt(mContext, --mARIAOwnsIdx);
      }

      if (!mAnchorNode) {
        mPhase = eAtStart;
        return nullptr;
      }

      mPhase = eAtDOM;
      PushState(mAnchorNode, false);
    }
  }

  dom::AllChildrenIterator* top = &mStateStack[mStateStack.Length() - 1];
  while (top) {
    while (nsIContent* childNode = top->GetPreviousChild()) {
      bool skipSubtree = false;

      // No accessible creation on the way back.
      Accessible* child = mDoc->GetAccessible(childNode);

      // Ignore the accessible and its subtree if it was repositioned by means of
      // aria-owns.
      if (child) {
        if (child->IsRelocated()) {
          continue;
        }
        return child;
      }

      // Walk down into subtree to find accessibles.
      if (!skipSubtree && childNode->IsElement()) {
        top = PushState(childNode, true);
      }
    }
    top = PopState();
  }

  // Move to a previous node relative the anchor node within the context
  // subtree if asked.
  if (mFlags != eWalkContextTree) {
    mPhase = eAtStart;
    return nullptr;
  }

  nsINode* contextNode = mContext->GetNode();
  while (mAnchorNode != contextNode) {
    nsINode* parentNode = mAnchorNode->GetFlattenedTreeParent();
    if (!parentNode || !parentNode->IsElement()) {
      return nullptr;
    }

    nsIContent* parent = parentNode->AsElement();
    top = PushState(parent, true);
    if (top->Seek(mAnchorNode)) {
      mAnchorNode = parent;
      return Prev();
    }

    mAnchorNode = parent;
  }

  mPhase = eAtStart;
  return nullptr;
}

dom::AllChildrenIterator*
TreeWalker::PopState()
{
  size_t length = mStateStack.Length();
  mStateStack.RemoveElementAt(length - 1);
  return mStateStack.IsEmpty() ? nullptr : &mStateStack[mStateStack.Length() - 1];
}
