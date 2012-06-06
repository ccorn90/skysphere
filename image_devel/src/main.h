#ifndef __MAIN_H__
#define __MAIN_H__
//From Capture.h in Blackmagic SDK

#include "DeckLinkAPI.h"
#include "DeckLinkAPIDispatch.cpp"

class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
  DeckLinkCaptureDelegate();
  ~DeckLinkCaptureDelegate();
  
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
  virtual ULONG STDMETHODCALLTYPE AddRef(void);
  virtual ULONG STDMETHODCALLTYPE  Release(void);
  virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
  virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);
  
 private:
  ULONG	m_refCount;
  pthread_mutex_t m_mutex;
};

#endif
