#ifndef SIMPLE_CHUNK
#define SIMPLE_CHUNK

namespace mozilla {
struct SimpleChunk {
  SimpleChunk() {}
  ~SimpleChunk() {}
  void SliceTo(TrackTicks aStart, TrackTicks aEnd)
  {
    NS_ASSERTION(aStart >= 0 && aStart < aEnd && aEnd <= mDuration,
                 "Slice out of bounds");
    mDuration = aEnd - aStart;
  }
  TrackTicks GetDuration() const { return mDuration; }
  bool CanCombineWithFollowing(const SimpleChunk& aOther) const
  {
    MOZ_CRASH("need impl");
    return false; //aOther.mFrame == mFrame;
  }
  bool IsNull() const { MOZ_CRASH("need impl"); return false;/*!mFrame.GetImage();*/ }
  void SetNull(TrackTicks aDuration)
  {
    mDuration = aDuration;
    //mFrame.SetNull();
    MOZ_CRASH("need impl");
    mTimeStamp = TimeStamp();
  }
  void SetForceBlack(bool aForceBlack) { MOZ_CRASH("need impl"); /*mFrame.SetForceBlack(aForceBlack);*/ }

  TrackTicks mDuration;
  //VideoFrame mFrame;
  mozilla::TimeStamp mTimeStamp;
};

class SimpleSegment : public MediaSegmentBase<SimpleSegment, SimpleChunk> {
public:
  typedef mozilla::layers::Image Image;

SimpleSegment() : MediaSegmentBase<SimpleSegment, SimpleChunk>(VIDEO) {}
  ~SimpleSegment() {}

  void AppendFrame(already_AddRefed<Image> aImage, TrackTicks aDuration,
                   const gfxIntSize& aIntrinsicSize) {}
/*  const VideoFrame* GetFrameAt(TrackTicks aOffset, TrackTicks* aStart = nullptr)
  {
    SimpleChunk* c = FindChunkContaining(aOffset, aStart);
    if (!c) {
      return nullptr;
    }
    return &c->mFrame;
  }
  const VideoFrame* GetLastFrame(TrackTicks* aStart = nullptr)
  {
    SimpleChunk* c = GetLastChunk();
    if (!c) {
      return nullptr;
    }
    if (aStart) {
      *aStart = mDuration - c->mDuration;
    }
    return &c->mFrame;
  }
*/
  // Override default impl
  virtual void ReplaceWithDisabled() MOZ_OVERRIDE {
    for (ChunkIterator i(*this);
         !i.IsEnded(); i.Next()) {
      SimpleChunk& chunk = *i;
      chunk.SetForceBlack(true);
    }
  }

  // Segment-generic methods not in MediaSegmentBase
  static Type StaticType() { return VIDEO; }
};

}

#endif
