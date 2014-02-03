/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TEST_PCOBSERVER_H_
#define TEST_PCOBSERVER_H_

#include "nsNetCID.h"
#include "nsITimer.h"
#include "nsComponentManagerUtils.h"
#include "nsIComponentManager.h"
#include "nsIComponentRegistrar.h"

#include "mozilla/Mutex.h"
#include "AudioSegment.h"
#include "MediaSegment.h"
#include "StreamBuffer.h"
#include "nsTArray.h"
#include "nsIRunnable.h"
#include "nsISupportsImpl.h"
#include "nsIDOMMediaStream.h"
#include "mozilla/dom/PeerConnectionObserverEnumsBinding.h"
#include "PeerConnectionImpl.h"
#include "nsWeakReference.h"

namespace sipcc {
class PeerConnectionImpl;
}

class nsIDOMWindow;
class nsIDOMDataChannel;

namespace test {
using namespace mozilla::dom;

class AFakePCObserver : public PeerConnectionObserverExternal
{
protected:
  typedef mozilla::ErrorResult ER;
public:
  enum Action {
    OFFER,
    ANSWER
  };

  enum ResponseState {
    stateNoResponse,
    stateSuccess,
    stateError
  };

  AFakePCObserver(sipcc::PeerConnectionImpl *peerConnection,
                  const std::string &aName) :
    state(stateNoResponse), addIceSuccessCount(0),
    onAddStreamCalled(false),
    name(aName),
    pc(peerConnection) {
  }

  virtual ~AFakePCObserver() {}

  std::vector<mozilla::DOMMediaStream *> GetStreams() { return streams; }

  ResponseState state;
  char *lastString;
  sipcc::PeerConnectionImpl::Error lastStatusCode;
  mozilla::dom::PCObserverStateType lastStateType;
  int addIceSuccessCount;
  bool onAddStreamCalled;
  std::string name;
  std::vector<std::string> candidates;

protected:
  sipcc::PeerConnectionImpl *pc;
  std::vector<mozilla::DOMMediaStream *> streams;
};
}

namespace mozilla {
namespace dom {
typedef PeerConnectionObserverExternal PeerConnectionObserver;
}
}

#endif
