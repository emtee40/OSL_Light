#include "StdAfx.h"
#include "ImageAdjuster.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_ImageAdjuster( 
	Eyw::block_class_registrant::block_id( "ImageAdjuster" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "ImageAdjuster" )
			.description( "Image Adjuster build 1.6.2.80\n Testing block for development only.\n"
							"Image list, second image output, delayed image stream, delta flow testing.\n" 
							"Pixelwise comparator, image bus, color channel compose, color fraction.\n"
							"New effects options testing - mirror, filters, buffer.\n"
							"This block provides little if any checks on input datatypes and functions.\n" )
			.libraries( "OSL" )
			.bitmap( IDB_IMAGEADJUSTER_BITMAP )
		.end_language()	
		.begin_authors()
			.author( EYW_OSL_LIGHT_CATALOG_AUTHOR_ID )
		.end_authors()
		.begin_companies()
			.company( EYW_OSL_LIGHT_COMPANY_ID )
		.end_companies()
		.begin_licences()
			.licence( EYW_OSL_LIGHT_LICENSE_ID )
		.end_licences()
		.default_factory< CImageAdjuster >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define INPUT_CENTER_X "InputCenterX"
#define INPUT_CENTER_Y "InputCenterY"
#define INPUT_SIZE_WIDE "InputSizeWide"
#define INPUT_SIZE_HIGH "InputSizeHigh"
#define INPUT_COLOR_DOUBLE "InputColorDouble"

#define INPUT_INPUTIMAGE "InputImage"
#define OUTPUT_OUTPUTIMAGE "OutputImage"
#define OUTPUT_IMAGE_BW "OutputImageBW"
//#define INPUT_AUDIO_IN "audioIn"


/////////////////////////////////
CImageAdjuster::CImageAdjuster( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInInputImage = NULL;
	_pOutOutputImage = NULL;
	_pOutputImageBW = NULL;

	//_pInAudioIn = NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_INPUTIMAGE, true );
}

CImageAdjuster::~CImageAdjuster()
{
}

void CImageAdjuster::InitSignature()
{
	_pCenterX = Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(INPUT_CENTER_X)
	                         .name("InputCenterX")
	                         .description("Parameter to adjust input image center position_x")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(360, true)
	                         )->GetDatatype() );

	_pCenterY = Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(INPUT_CENTER_Y)
	                         .name("InputCenterY")
	                         .description("Parameter to adjust input image center position_y")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(288, true)
	                         )->GetDatatype() );

	_pSizeWide = Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(INPUT_SIZE_WIDE)
	                         .name("InputSizeWide")
	                         .description("Parameter to adjust size")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(3, true)
							 .max(11, true)
	                         )->GetDatatype() );
	
	_pSizeHigh = Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(INPUT_SIZE_HIGH)
	                         .name("InputSizeHigh")
	                         .description("Parameter to adjust input image height")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(1, true)
							 .max(576, true)
	                         )->GetDatatype() );
	
	_pColorDouble= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(INPUT_COLOR_DOUBLE)
	                         .name("InputColorDouble")
	                         .description("double value of color fraction")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );

	SetInput(Eyw::pin::id(INPUT_INPUTIMAGE)
	    .name("InputImage")
	    .description("Input image stream")
	    .type<Eyw::IImage>()
	    );
	/*
	SetInput(Eyw::pin::id(INPUT_AUDIO_IN)
	    .name("AudioIn")
	    .description("Input audio stream")
	    .type<Eyw::IAudioBuffer>()
	    );
	*/
	SetOutput(Eyw::pin::id(OUTPUT_OUTPUTIMAGE)
	    .name("OutputImage")
	    .description("Adjusted output image")
	    .type<Eyw::IImage>()
		//.inplace_id( INPUT_INPUTIMAGE )
	    );

	SetOutput(Eyw::pin::id(OUTPUT_IMAGE_BW)
	    .name("OutputImageBW")
	    .description("BW output image")
	    .type<Eyw::IImage>()
	    );	

	// default values
	_pCenterX->SetValue( 0 );
	_pCenterY->SetValue( 0 );
	_pSizeWide->SetValue( 3 );
	_pSizeHigh->SetValue( 1 );
	_pColorDouble->SetValue( 0.0 );
}

void CImageAdjuster::CheckSignature()
{
	_pCenterX = get_parameter_datatype<Eyw::IInt>(INPUT_CENTER_X);
	_pCenterY = get_parameter_datatype<Eyw::IInt>(INPUT_CENTER_Y);

	_pSizeWide = get_parameter_datatype<Eyw::IInt>(INPUT_SIZE_WIDE);
	_pSizeHigh = get_parameter_datatype<Eyw::IInt>(INPUT_SIZE_HIGH);

	_pColorDouble = get_parameter_datatype<Eyw::IDouble>(INPUT_COLOR_DOUBLE);

	_signaturePtr->GetInputs()->FindItem( INPUT_INPUTIMAGE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_OUTPUTIMAGE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_IMAGE_BW );

	//_signaturePtr->GetInputs()->FindItem( INPUT_AUDIO_IN );
}

void CImageAdjuster::DoneSignature()
{
	_pCenterX = NULL;
	_pCenterY = NULL;
	_pSizeWide = NULL;
	_pSizeHigh = NULL;
	_pColorDouble = NULL;
}

// Actions
bool CImageAdjuster::Init() throw()
{
    try
    {
		Notify_MessageString( "\nImageAdjuster block: Init begin....\n" );
		_pInInputImage = get_input_datatype<Eyw::IImage>( INPUT_INPUTIMAGE );
		_pOutOutputImage = get_output_datatype<Eyw::IImage>( OUTPUT_OUTPUTIMAGE );
		_pOutputImageBW = get_output_datatype<Eyw::IImage>( OUTPUT_IMAGE_BW );

		inInputImageInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inInputImageInitInfoPtr->CopyFrom( _pInInputImage->GetInitInfo() );

		/*
		if( ( _pInInputImage->GetColorModel() != ecmBW ) ) {
			// TODO properly...
			inInputImageInitInfoPtr->SetColorModel( ecmBW );
			//_pInInputImage->ConvertColorModel( _pInInputImage.get() );
			//_pInInputImage->InitInstance( inInputImageInitInfoPtr.get() );
			Notify_MessageString( "\nImageAdjuster block: Source Color model not Black and White, converting ecmBW.\n" );
		}
		*/

		outOutputImageInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		outOutputImageInitInfoPtr->CopyFrom( _pInInputImage->GetInitInfo() );
		//outOutputImageInitInfoPtr->SetWidth( 240 ); //716
		//outOutputImageInitInfoPtr->SetHeight( 192 ); //572
		_pOutOutputImage->InitInstance( outOutputImageInitInfoPtr.get() );

		// setup second output (Blk & Wht) image
		outputImageBWInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( outputImageBWInitInfoPtr, inInputImageInitInfoPtr );
		outputImageBWInitInfoPtr->SetColorModel( ecmBW );
		_pOutputImageBW->InitInstance( outputImageBWInitInfoPtr.get() );
		
		// setup temp image
		tempImageInitInfo = datatype_init_info<IImageInitInfo>::create( GetKernelServices() );
		copy_datatype_init_info( tempImageInitInfo, inInputImageInitInfoPtr );
		tempImagePtr = datatype<IImage>::create( GetKernelServices() );
		tempImagePtr->InitInstance( tempImageInitInfo.get() );
		
		
		// setup adjust image, copy BW
		adjustImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( adjustImageInitInfo, outputImageBWInitInfoPtr );
		adjustImagePtr = datatype<IImage>::create( GetKernelServices() );
		adjustImageInitInfo->SetColorModel( ecmBW );
		adjustImagePtr->InitInstance( adjustImageInitInfo.get() );

		/*
		// setup bus1 image
		bus1ImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( bus1ImageInitInfo, inInputImageInitInfoPtr );
		bus1ImagePtr = datatype<IImage>::create( GetKernelServices() );
		bus1ImagePtr->InitInstance( bus1ImageInitInfo.get() );

		// setup bus2 image
		bus2ImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( bus2ImageInitInfo, inInputImageInitInfoPtr );
		bus2ImagePtr = datatype<IImage>::create( GetKernelServices() );
		bus2ImagePtr->InitInstance( bus2ImageInitInfo.get() );

		// other vars here
		origWidth = 720;
		origHeight = 576;
		tempX = 0;
		tempY = 0;
		pixelModel = 4; // unknown
		*/
		/*
		delayTime = 2;
		debugCounter = 0;
		colorParam = 0.0;
		// init the list for delayed image
		listInitInfo = Eyw::datatype_init_info<Eyw::IListInitInfo>::create(_kernelServicesPtr);
		listInitInfo->SetCatalogID( Eyw::datatype_traits<Eyw::IImage>::get_catalog_id() );
		listInitInfo->SetClassID( Eyw::datatype_traits<Eyw::IImage>::get_class_id() );
		imageList = datatype<Eyw::IList>::create( GetKernelServices() );
		imageList->InitInstance(listInitInfo.get());
		imageList->Clear(); // needed here?

		// init the list for buffered images(3-4), same initInfo as above list
		bufferList = datatype<Eyw::IList>::create( GetKernelServices() );
		bufferList->InitInstance(listInitInfo.get());
		bufferList->Resize(4); // buffer window size/length
		bufferList->Clear(); // needed here?
		
		//
		_pInAudioInInitInfo = Eyw::datatype_init_info<Eyw::IAudioBufferInitInfo>::create( _kernelServicesPtr );;
		_pInAudioInInitInfo->CopyFrom( _pInAudioIn->GetInitInfo() );
		//_pInAudioIn->InitInstance( _pInAudioInInitInfo.get() );
		*/

		frameCounter = 0;
		bufferCount = 4;
		Notify_MessageString( "ImageAdjuster block: Init end OK.\n" );

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CImageAdjuster::Start() throw()
{
    try
    {
    	// TODO: add your logic
		// run once at start method...
		Notify_MessageString( "ImageAdjuster block: Start begin....\n" );
		/*
		// setup reSizing:
		topLeft._x = 0;
		topLeft._y = 0;
		srcSize._cx = 360;
		srcSize._cy = 288;
		srcRect._size = srcSize;
		srcRect._origin = topLeft;

		destSize._cx = 720;
		destSize._cy = 576;
		destRect._size = destSize;
		destRect._origin = topLeft;

		// this adds a bottom layer of red fill
		rgbColor._red = 1; // 0.0 - 1.0
		rgbColor._blue = 0;
		rgbColor._green = 0;
		alpha = 0.1;

		// rotate
		center._x = 360;
		center._y = 288;
		angle = 180.0;

		interpolate = eiLinear;
		letterBox = elmNone;

		sizeParam = 3;
		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;
		anchorSize._x = 1; 
		anchorSize._y = 1;
		*/
		Notify_MessageString("ImageAdjuster block: Start end OK.\n");
    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CImageAdjuster::Execute() throw()
{
    try
    {
		/*
		THIS METHOD RUNS AT FPS RATE...
		*/
		Notify_MessageString( "ImageAdjuster block: execute begin....\n" );
		// grab input image and copy to tempImage, manipulate, copy to output pin

		//tempImagePtr->SetColor( rgbColor, alpha );
		//tempImagePtr->CopySingleChannel( _pInInputImage.get(), 2 );  //0, 1, 2 = b, g, r
		//copy_datatype_info( tempImagePtr, _pInInputImage );

		
		// grab parameter changes for centerXY 
		// need sanity checks here or CRASH!!
		// check range is within limits and then 
		// change topLeft
		// else keep current value ( not goto 0 )
		/*
		// CENTER POSITION
		if (_pCenterX->GetValue() >= 0 && _pCenterX->GetValue() <= (origWidth / 2)) {
			topLeft._x = _pCenterX->GetValue();
		}
		else {
			// do nothing
		}
		if (_pCenterY->GetValue() >= 0 && _pCenterY->GetValue() <= (origHeight / 2)) {
			topLeft._y = _pCenterY->GetValue();
		}
		else {
			//do nothing
		}
		// apply them
		srcRect._origin = topLeft;

		
		// SIZE RECT
		if (_pSizeWide->GetValue() >= 1 && _pSizeWide->GetValue() <= origWidth) {
			srcSize._cx = _pSizeWide->GetValue();
		}
		else {
			// do nothing
		}
		if (_pSizeHigh->GetValue() >= 1 && _pSizeHigh->GetValue() <= origHeight) {
			srcSize._cy = _pSizeHigh->GetValue();
		}
		else {
			// do nothing
		}
		// apply them
		srcRect._size = srcSize;
		// call resize
		tempImagePtr->Resize( _pInInputImage.get(), srcRect, destRect, interpolate );

		// copy tempImage to output pin
		_pOutOutputImage->CopyFrom( tempImagePtr.get() );
		_pOutOutputImage->SetCreationTime( _clockPtr->GetTime() );

		// pixelModel - orig: epm16s , 16bits per channel signed
		pixelModel = _pInInputImage->GetColorModel();
		*/

// SOMETHING OLD
		
		/*
		// start sending to list
		imageList->PushFront( _pInInputImage.get() );

		if (frameCounter >= delayTime ) {
			// start copyfrom list
			adjustImagePtr->CopyFrom( imageList->Back() );
			imageList->PopBack();
		}
		
		sizeParam = _pSizeWide->GetValue();
		if (sizeParam < 3 || sizeParam > 11) {
			sizeParam = 3;
		}
		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;

		// copy orig first
		tempImagePtr->CopyFrom( _pInInputImage.get() );
		
		// comparator
		tempImagePtr->PixelwiseDifferentFrom( adjustImagePtr.get() );

		// grab tempImage in current state
		bus1ImagePtr->CopyFrom( tempImagePtr.get() );


		bus2ImagePtr->CopyFrom( tempImagePtr.get() );
		
		// subtract
		//bus1ImagePtr->SubtractAbsolute( tempImagePtr.get() );
		// gain...?
		bufferList->PushFront( bus1ImagePtr.get() );

		//bus1ImagePtr->Add( bus2ImagePtr.get() );
		//bus2ImagePtr->CopyFrom( bus1ImagePtr.get() );

		// compose bgr from 3 greyscale images
		//0, 1, 2 = b, g, r
		Notify_MessageString( "ImageAdjuster block: output ecm: %d.\n",  _pOutOutputImage->GetColorModel() );

		colorParam = _pColorDouble->GetValue();
		if (colorParam < 0.0 || colorParam > 1.0) {
			// invalid value
			colorParam = 0.5;
		}

		_pOutOutputImage->CopySingleChannel( (Eyw::IImage*)bufferList->Front(), 0 );
		_pOutOutputImage->SubtractDouble( colorParam ); // only from blue channel?

		if (bufferList->Size() >= 3) {
			//
			//bus2ImagePtr->Add( bufferList->Back() );
			_pOutOutputImage->CopySingleChannel( (Eyw::IImage*)bufferList->Back(), 1 );
			bufferList->PopBack();
			//bus2ImagePtr->Add( bufferList->Back() );
			_pOutOutputImage->CopySingleChannel( (Eyw::IImage*)bufferList->Back(), 2 );
		}

		bus2ImagePtr->SubtractDouble( colorParam ); // 1.0 equals no image

		_pOutOutputImage->FilterMedian( _pOutOutputImage.get(), maskSize, anchorSize );	
		_pOutOutputImage->SetCreationTime( _clockPtr->GetTime() );

		*/

		// hipass has a double matrix for the kernel..
		// 3x3 is 3 rows 3 cols
		// and an int for the divisor
		// fixed kernel filters do not need configuring, other than either setting ROI to be within the border pixels
		// or to set border pixels for the image. 
		// this is to ensure algorithm does not look beyond image boundary for neighbouring pixels..


		//_pOutOutputImage->CopyFrom(_pInInputImage.get() );
		adjustROI = _pInInputImage->GetOrigROI();

		sizeROI = adjustROI._size;
		originROI = adjustROI._origin;

		// adjust them here to allow for border pixels
		originROI._x = 4;
		originROI._y = 4;
		sizeROI._cx = 712; 
		sizeROI._cy = 568;

		// set them, don't forget...
		adjustROI._origin = originROI;
		adjustROI._size = sizeROI;

		Notify_MessageString( "ImageAdjuster block: adjustROI size x, y: %d, %d\n", adjustROI._size._cx, adjustROI._size._cy );
		Notify_MessageString( "ImageAdjuster block: adjustROI origin x, y: %d, %d\n", adjustROI._origin._x, adjustROI._origin._y );

		adjustImagePtr->ConvertColorModel( _pInInputImage.get() );
		adjustImagePtr->SetROI( adjustROI );

		
		//_pOutputImageBW->FilterHipass5x5( adjustImagePtr.get() );
		//_pOutputImageBW->SetROI( adjustROI );
		_pOutputImageBW->CopyFrom( adjustImagePtr.get() );
		_pOutputImageBW->SetCreationTime( _clockPtr->GetTime() );

		//_pOutOutputImage->FilterHipass5x5( _pInInputImage.get() );
		tempImagePtr->SetROI( adjustROI );
		tempImagePtr->CopyFromBuffer( adjustImagePtr->GetROIBuffer(), adjustImagePtr->GetStepSize() );
		

		tempROI = tempImagePtr->GetROI();
		Notify_MessageString( "ImageAdjuster block: tempROI size x, y: %d, %d\n", tempROI._size._cx, tempROI._size._cy );
		Notify_MessageString( "ImageAdjuster block: tempROI origin x, y: %d, %d\n", tempROI._origin._x, tempROI._origin._y );

		//_pOutOutputImage->FilterHipass5x5( tempImagePtr.get() );
		_pOutOutputImage->CopyFrom( tempImagePtr.get() );

		_pOutOutputImage->SetCreationTime( _clockPtr->GetTime() );
		//
		
		/*
		// epm == epm8u
		// filters generally require 8u16s_C1R (single channel)
		adjustImagePtr->ConvertColorModel( _pInInputImage.get() );

		if (frameCounter <= bufferCount) {
			// do nothing but count
			frameCounter++;
		}
		else {
			// epm == epm8u
			// filters generally require 8u16s_C1R (single channel)
			//adjustImagePtr->ConvertColorModel( _pInInputImage.get() );
			// filtered copy
			//tempImagePtr->CopyFromBuffer( adjustImagePtr->GetBuffer(), adjustImagePtr->GetStepSize() );

			_pOutputImageBW->FilterHipass3x3( adjustImagePtr.get() );
			//_pOutputImageBW->CopyFrom( adjustImagePtr.get() );
			_pOutputImageBW->SetCreationTime( _clockPtr->GetTime() );
		}
		*/
		// debug output
		//debugCounter++;
		//Notify_MessageString( "Execute.frameCounter: %d\n", frameCounter );
		Notify_MessageString( "ImageAdjuster block: Execute end OK.\n" );
    }
    catch(...)
    {
    }
	return true;
}

void CImageAdjuster::Stop() throw()
{
    try
    {
		// reset some vars here when patch stopped
		frameCounter = 0;
		//imageList->Clear();
    }
    catch(...)
    {
    }
}

void CImageAdjuster::Done() throw()
{
    try
    {
		// release resources
		_pInInputImage = NULL;
		_pOutOutputImage = NULL;
		_pOutputImageBW = NULL;
		/*
		tempImagePtr = NULL;
		adjustImagePtr = NULL;
		bus1ImagePtr = NULL;
		bus2ImagePtr = NULL;
		*/

    }
    catch(...)
    {
    }
}

// helper function
double CImageAdjuster::Round(long dbVal)
{
    const double dbShift = pow(10.0, 2);
    return  floor(dbVal * dbShift + 0.5) / dbShift; 
}

void CImageAdjuster::AdjustImage()
{
	//TODO
	// add some code to change some elements of the input image,
	// copy it to the tempImage
	// allow output pin to copy it.
	//SIZE_2D size_in = tempImagePtr->GetSize();
}
