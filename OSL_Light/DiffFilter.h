#pragma once

class CDiffFilter : public Eyw::CBlockImpl
{
public:
	CDiffFilter( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CDiffFilter();

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
	virtual void debugStuff();

private:
	Eyw::trigger_ptr _pInTrigger;
	Eyw::int_ptr _pInThresholdLower;
	Eyw::int_ptr _pInSizeInt;
	Eyw::image_ptr _pInSource;
	Eyw::image_ptr _pOutFinalImage;

	Eyw::image_init_info_ptr inSourceInitInfoPtr;
	Eyw::image_init_info_ptr outFinalImageInitInfoPtr;

	Eyw::image_ptr convertImagePtr;
	Eyw::image_init_info_ptr convertImageInitInfo;

	Eyw::image_ptr tempImagePtr;
	Eyw::image_init_info_ptr tempImageInitInfo;

	Eyw::image_ptr snapshotImagePtr;
	Eyw::image_init_info_ptr snapshotImageInitInfo;

	int sizeParam;
	Eyw::SIZE_2D maskSize;
	Eyw::POINT_2D anchorSize;

	int minLowerThresholdValue;
	int maxUpperThresholdValue;
	int lowerValueParam;
	int upperValueParam;

	bool convertCM;
	int debugColorModel;

};
