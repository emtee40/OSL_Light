#pragma once

class CDelayColor : public Eyw::CBlockImpl
{
public:
	CDelayColor( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CDelayColor();

protected:
	virtual void InitSignature();	// should also initialize layout and private data
	virtual void CheckSignature();
	virtual void DoneSignature();

	// Actions
	virtual bool Init() throw();
	virtual bool Start() throw();
	virtual bool Execute() throw();
	virtual void Stop() throw();
	virtual void Done() throw();

	virtual void OnChangedParameter(const std::string &parameterId);

private:
	Eyw::int_ptr _pParamInDelay1;
	Eyw::int_ptr _pParamInDelay2;

	Eyw::double_ptr _pParamInFractionB;
	Eyw::double_ptr _pParamInFractionG;
	Eyw::double_ptr _pParamInFractionR;

	Eyw::image_ptr _pInSource;
	Eyw::image_ptr _pOutFinal;

	Eyw::image_init_info_ptr inSourceInitInfoPtr;
	Eyw::image_init_info_ptr outFinalInitInfoPtr;

	Eyw::list_init_info_ptr bufferListInitInfo;
	Eyw::list_ptr buffer1List;
	Eyw::list_ptr buffer2List;

	Eyw::image_ptr channelB;
	Eyw::image_ptr channelG;
	Eyw::image_ptr channelR;
	Eyw::image_init_info_ptr channelBGRInitInfo;

	int frameCounter;
	int buffer1Count;
	int lastBuffer1Count;
	int buffer2Count;
	int lastBuffer2Count;
	int maxCounter;
	double paramB;
	double paramG;
	double paramR;

};
