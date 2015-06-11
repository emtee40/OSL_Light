#pragma once

class CImageAdjuster : public Eyw::CBlockImpl
{
public:
	CImageAdjuster( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CImageAdjuster();

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

	double Round(long dbVal);
	void AdjustImage();

private:
	Eyw::int_ptr _pCenterX;
	Eyw::int_ptr _pCenterY;

	Eyw::int_ptr _pSizeWide;
	Eyw::int_ptr _pSizeHigh;

	Eyw::double_ptr _pColorDouble;

	Eyw::image_ptr _pInInputImage;
	Eyw::image_ptr _pOutOutputImage;
	Eyw::image_ptr _pOutputImageBW;

	
	Eyw::image_ptr tempImagePtr;
	Eyw::image_init_info_ptr tempImageInitInfo;

	
	Eyw::image_ptr adjustImagePtr;
	Eyw::image_init_info_ptr adjustImageInitInfo;
	/*
	Eyw::image_ptr bus1ImagePtr;
	Eyw::image_init_info_ptr bus1ImageInitInfo;

	Eyw::image_ptr bus2ImagePtr;
	Eyw::image_init_info_ptr bus2ImageInitInfo;
	*/

	Eyw::image_init_info_ptr outOutputImageInitInfoPtr;
	Eyw::image_init_info_ptr inInputImageInitInfoPtr;
	Eyw::image_init_info_ptr outputImageBWInitInfoPtr;
	
	Eyw::RECT_2D adjustROI;
	Eyw::RECT_2D tempROI;
	Eyw::SIZE_2D_INT sizeROI;
	Eyw::POINT_2D_INT originROI;

	Eyw::ROI_2D royBoy;

	int frameCounter;
	int bufferCount;
	/*
	int delayTime;
	int debugCounter;
	double colorParam;
	
	Eyw::list_init_info_ptr listInitInfo;
	Eyw::list_ptr imageList;
	Eyw::list_ptr bufferList;

	size_t origWidth;
	size_t origHeight;

	Eyw::RGBCOLOR rgbColor;
	double alpha;

	int pixelModel;

	Eyw::POINT_2D_DOUBLE center;
	double angle;

	Eyw::RECT_2D srcRect;
	Eyw::POINT_2D_INT topLeft;
	Eyw::SIZE_2D_INT srcSize;
	Eyw::RECT_2D destRect;
	Eyw::SIZE_2D_INT destSize;
	Eyw::EInterpolation interpolate;
	Eyw::ELetterboxMode letterBox;

	int tempX;
	int tempY;

	int sizeParam;
	Eyw::SIZE_2D maskSize;
	Eyw::POINT_2D anchorSize;
	
	Eyw::audio_buffer_ptr _pInAudioIn;
	Eyw::audio_buffer_init_info_ptr _pInAudioInInitInfo;
	*/
};
