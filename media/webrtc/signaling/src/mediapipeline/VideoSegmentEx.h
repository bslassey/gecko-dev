/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_VIDEOSEGMENTEX_H_
#define MOZILLA_VIDEOSEGMENTEX_H_

#include "MediaSegment.h"
#include "nsCOMPtr.h"
#include "gfxPoint.h"
#include "nsAutoPtr.h"
#include "ImageContainer.h"

namespace mozilla {

class VideoFrameEx {
public:

  VideoFrameEx(const unsigned char* aImage, unsigned int aImageSize, const gfxIntSize& aIntrinsicSize);
  VideoFrameEx();
  ~VideoFrameEx();

  bool operator==(const VideoFrameEx& aFrame) const
  {
    return mIntrinsicSize == aFrame.mIntrinsicSize &&
           mForceBlack == aFrame.mForceBlack &&
           ((mForceBlack && aFrame.mForceBlack) || (mImage == aFrame.mImage ));
  }
  bool operator!=(const VideoFrameEx& aFrame) const
  {
    return !operator==(aFrame);
  }

  const unsigned char* GetImage(unsigned int* aImageSize) const { if (aImageSize) { *aImageSize = mImageSize; } return mImage; }
  void SetForceBlack(bool aForceBlack) { mForceBlack = aForceBlack; }
  bool GetForceBlack() const { return mForceBlack; }
  const gfxIntSize& GetIntrinsicSize() const { return mIntrinsicSize; }
  void SetNull();
  void TakeFrom(VideoFrameEx* aFrame);

protected:
  // mImage can be null to indicate "no video" (aka "empty frame"). It can
  // still have an intrinsic size in this case.
  const unsigned char* mImage;
  unsigned int mImageSize;
  // The desired size to render the video frame at.
  gfxIntSize mIntrinsicSize;
  bool mForceBlack;
};

struct VideoChunkEx {
  VideoChunkEx();
  ~VideoChunkEx();
  void SliceTo(TrackTicks aStart, TrackTicks aEnd)
  {
    NS_ASSERTION(aStart >= 0 && aStart < aEnd && aEnd <= mDuration,
                 "Slice out of bounds");
    mDuration = aEnd - aStart;
  }
  TrackTicks GetDuration() const { return mDuration; }
  bool CanCombineWithFollowing(const VideoChunkEx& aOther) const
  {
    return aOther.mFrame == mFrame;
  }
  bool IsNull() const { return !mFrame.GetImage(nullptr); }
  void SetNull(TrackTicks aDuration)
  {
    mDuration = aDuration;
    mFrame.SetNull();
    mTimeStamp = TimeStamp();
  }
  void SetForceBlack(bool aForceBlack) { mFrame.SetForceBlack(aForceBlack); }

  TrackTicks mDuration;
  VideoFrameEx mFrame;
  mozilla::TimeStamp mTimeStamp;
};

class VideoSegmentEx : public MediaSegmentBase<VideoSegmentEx, VideoChunkEx> {
public:
  VideoSegmentEx();
  ~VideoSegmentEx();

  void AppendFrame(const unsigned char* aImage, unsigned int aImageSize,
                   TrackTicks aDuration, const gfxIntSize& aIntrinsicSize);
  const VideoFrameEx* GetFrameAt(TrackTicks aOffset, TrackTicks* aStart = nullptr)
  {
    VideoChunkEx* c = FindChunkContaining(aOffset, aStart);
    if (!c) {
      return nullptr;
    }
    return &c->mFrame;
  }
  const VideoFrameEx* GetLastFrame(TrackTicks* aStart = nullptr)
  {
    VideoChunkEx* c = GetLastChunk();
    if (!c) {
      return nullptr;
    }
    if (aStart) {
      *aStart = mDuration - c->mDuration;
    }
    return &c->mFrame;
  }
  // Override default impl
  virtual void ReplaceWithDisabled() MOZ_OVERRIDE {
    for (ChunkIterator i(*this);
         !i.IsEnded(); i.Next()) {
      VideoChunkEx& chunk = *i;
      chunk.SetForceBlack(true);
    }
  }

  // Segment-generic methods not in MediaSegmentBase
  static Type StaticType() { return VIDEO; }
};

}

#endif /* MOZILLA_VIDEOSEGMENTEX_H_ */
