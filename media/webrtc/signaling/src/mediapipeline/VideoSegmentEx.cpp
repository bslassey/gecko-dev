/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "VideoSegmentEx.h"
#include "ImageContainer.h"

#include <stdio.h>

namespace mozilla {

VideoFrameEx::VideoFrameEx(const unsigned char* aImage, unsigned int aImageSize, const gfxIntSize& aIntrinsicSize)
  : mImage(aImage)
  , mImageSize(aImageSize)
  , mIntrinsicSize(aIntrinsicSize)
  , mForceBlack(false)
{}

VideoFrameEx::VideoFrameEx()
  : mImage(nullptr), mImageSize(0), mIntrinsicSize(0, 0), mForceBlack(false)
{}

VideoFrameEx::~VideoFrameEx()
{
  SetNull();
}

void
VideoFrameEx::SetNull() {
  delete mImage;
  mImage = nullptr;
  mImageSize = 0;
  mIntrinsicSize = gfxIntSize(0, 0);
}

void
VideoFrameEx::TakeFrom(VideoFrameEx* aFrame)
{
  mImage = aFrame->mImage;
  aFrame->mImage = nullptr;
  mImageSize = aFrame->mImageSize;
  aFrame->mImageSize = 0;
  mIntrinsicSize = aFrame->mIntrinsicSize;
  mForceBlack = aFrame->GetForceBlack();
}

VideoChunkEx::VideoChunkEx()
{}

VideoChunkEx::~VideoChunkEx()
{}

void
VideoSegmentEx::AppendFrame(const unsigned char* aImage, unsigned int aImageSize, TrackTicks aDuration,
                          const gfxIntSize& aIntrinsicSize)
{
  VideoChunkEx* chunk = AppendChunk(aDuration);
  VideoFrameEx frame(aImage,aImageSize, aIntrinsicSize);
  chunk->mFrame.TakeFrom(&frame);
}

VideoSegmentEx::VideoSegmentEx()
  : MediaSegmentBase<VideoSegmentEx, VideoChunkEx>(VIDEO)
{}

VideoSegmentEx::~VideoSegmentEx()
{}

}
