#pragma once

class CFlow : public Eyw::CBlockImpl
{
public:
	CFlow( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CFlow();

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
	Eyw::int_ptr _pParamSize;
	Eyw::double_ptr _pParamFractionB;
	Eyw::double_ptr _pParamFractionG;
	Eyw::double_ptr _pParamFractionR;
	Eyw::image_ptr _pInSource;
	Eyw::image_ptr _pOutFinalOut;

	Eyw::image_init_info_ptr inSourceInitInfoPtr;
	Eyw::image_init_info_ptr outFinalOutInitInfoPtr;

	bool convertCM;
	Eyw::image_ptr convertImagePtr;
	Eyw::image_init_info_ptr convertImageInitInfo;

	Eyw::image_ptr tempImagePtr;
	Eyw::image_init_info_ptr tempImageInitInfo;

	Eyw::image_ptr adjustImagePtr;
	Eyw::image_init_info_ptr adjustImageInitInfo;

	Eyw::image_ptr channelB;
	Eyw::image_ptr channelG;
	Eyw::image_ptr channelR;
	Eyw::image_init_info_ptr channelBGRInitInfo;

	int frameCounter;
	int delayTime;
	int bufferCount;

	double paramB;
	double paramG;
	double paramR;

	int sizeParam;
	Eyw::SIZE_2D maskSize;
	Eyw::POINT_2D anchorSize;

	Eyw::list_init_info_ptr listInitInfo;
	Eyw::list_ptr imageList;
	Eyw::list_ptr bufferList;

};
