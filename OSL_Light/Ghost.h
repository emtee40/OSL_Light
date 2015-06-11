#pragma once

class CGhost : public Eyw::CBlockImpl
{
public:
	CGhost( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CGhost();

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
	Eyw::int_ptr _pInGamma;
	Eyw::int_ptr _pParamThresholdLower;

	Eyw::image_ptr _pInSource;
	Eyw::image_init_info_ptr inSourceInitInfoPtr;

	Eyw::image_ptr _pOutFinalMix;
	Eyw::image_init_info_ptr outFinalMixInitInfoPtr;

	Eyw::image_ptr convertImagePtr;
	Eyw::image_init_info_ptr convertImageInitInfo;

	Eyw::image_ptr bufferImage;
	Eyw::image_init_info_ptr bufferImageInitInfo;

	Eyw::image_ptr bus1Image;
	Eyw::image_init_info_ptr bus1ImageInitInfo;

	Eyw::image_ptr bus2Image;
	Eyw::image_init_info_ptr bus2ImageInitInfo;

	Eyw::image_ptr blackScreen;
	Eyw::image_init_info_ptr blackScreenInitInfo;

	int frameCounter;
	int delayTime;
	bool convertCM;

	int minLowerThresholdValue;
	int maxUpperThresholdValue;
	int lowerValueParam;
	int upperValueParam;
	int gammaParam;

	Eyw::list_init_info_ptr listInitInfo;
	Eyw::list_ptr imageList;

};
