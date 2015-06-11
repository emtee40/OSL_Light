#pragma once

class CBusMixer : public Eyw::CBlockImpl
{
public:
	CBusMixer( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CBusMixer();

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

private:
	Eyw::double_ptr _pInMixerValue;
	Eyw::image_ptr _pInSource1;
	Eyw::image_ptr _pInSource2;
	Eyw::image_ptr _pOutFinalMix;

	Eyw::image_init_info_ptr inSource1InitInfoPtr;
	Eyw::image_init_info_ptr inSource2InitInfoPtr;
	Eyw::image_init_info_ptr outFinalMixInitInfoPtr;

	Eyw::image_ptr tempImagePtr;
	Eyw::image_init_info_ptr tempImageInitInfo;

	bool source2Present;
	double defaultMixerValue;
	double paramMixerValue;
	double paramMixerValueInvert;


};
