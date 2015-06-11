#include "StdAfx.h"
#include "Ghost.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_Ghost( 
	Eyw::block_class_registrant::block_id( "Ghost" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "Ghost" )
			.description( "Ghost build 1.0.6.11\nDelayed ghost of a BW image stream.\n"
							"Utilises an image list buffer, ecmBW convert, frame counter, threshold operation, internal image bus system, IArith max.\n" 
							"This block requires a black and white image but will provide internal conversion if needed. " )
			.libraries( "OSL" )
			.bitmap( IDB_GHOST_BITMAP )
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
		.default_factory< CGhost >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define PARAMETER_INGAMMA "inGamma"
#define PARAMETER_PARAMTHRESHOLDLOWER "paramThresholdLower"

#define INPUT_INSOURCE "inSource"
#define OUTPUT_OUTFINALMIX "outFinalMix"


/////////////////////////////////
CGhost::CGhost( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource=NULL;
	_pOutFinalMix=NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_INSOURCE, true );

}

CGhost::~CGhost()
{
}

void CGhost::InitSignature()
{
	_pInGamma= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_INGAMMA)
	                         .name("inGamma")
	                         .description("Gamma difference")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(255, true)
	                         )->GetDatatype() );
	_pParamThresholdLower= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_PARAMTHRESHOLDLOWER)
	                         .name("paramThresholdLower")
	                         .description("integer threshold lower value")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(63, true)
	                         )->GetDatatype() );

	SetInput(Eyw::pin::id(INPUT_INSOURCE)
	    .name("inSource")
	    .description("video source")
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_OUTFINALMIX)
	    .name("outFinalMix")
	    .description("output image of final mix")
	    .type<Eyw::IImage>()
	    );

	_pParamThresholdLower->SetValue( 18 );
	_pInGamma->SetValue( 32 );
}

void CGhost::CheckSignature()
{
	_pInGamma=get_parameter_datatype<Eyw::IInt>(PARAMETER_INGAMMA);
	_pParamThresholdLower=get_parameter_datatype<Eyw::IInt>(PARAMETER_PARAMTHRESHOLDLOWER);

	_signaturePtr->GetInputs()->FindItem( INPUT_INSOURCE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_OUTFINALMIX );

}

void CGhost::DoneSignature()
{
	_pInGamma = NULL;
	_pParamThresholdLower = NULL;
}

// Actions
bool CGhost::Init() throw()
{
    try
    {
		convertCM = false;

		_pInSource = get_input_datatype<Eyw::IImage>( INPUT_INSOURCE );
		inSourceInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inSourceInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );

		// check for ecmBW
		if( ( inSourceInitInfoPtr->GetColorModel() != ecmBW ) )
		{
			Notify_MessageString( "\nGhost block:: Source colorModel is not ecmBW, creating ecmBW convertor.\n" );
			inSourceInitInfoPtr->SetColorModel( ecmBW );
			convertImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
			copy_datatype_init_info( convertImageInitInfo, inSourceInitInfoPtr );
			convertImagePtr = datatype<IImage>::create( GetKernelServices() );
			convertImagePtr->InitInstance( convertImageInitInfo.get() );
			convertCM = true;
		}
		else 
		{
			Notify_MessageString( "\nGhost block: Source colorModel is ecmBW.\n" );
			convertCM = false;
			convertImageInitInfo = NULL;
			convertImagePtr = NULL;
		}
		
		// set output to be resultant color mix
		_pOutFinalMix = get_output_datatype<Eyw::IImage>( OUTPUT_OUTFINALMIX );
		outFinalMixInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( outFinalMixInitInfoPtr, inSourceInitInfoPtr );
		//outFinalMixInitInfoPtr->SetColorModel( ecmBGR );
		_pOutFinalMix->InitInstance( outFinalMixInitInfoPtr.get() );

		// setup buffer image
		bufferImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( bufferImageInitInfo, inSourceInitInfoPtr );
		bufferImage = datatype<IImage>::create( GetKernelServices() );
		bufferImage->InitInstance( bufferImageInitInfo.get() );

		// init the list for buffered images
		listInitInfo = Eyw::datatype_init_info<Eyw::IListInitInfo>::create(_kernelServicesPtr);
		listInitInfo->SetCatalogID( Eyw::datatype_traits<Eyw::IImage>::get_catalog_id() );
		listInitInfo->SetClassID( Eyw::datatype_traits<Eyw::IImage>::get_class_id() );
		imageList = datatype<Eyw::IList>::create( GetKernelServices() );
		imageList->InitInstance(listInitInfo.get());
		imageList->Clear(); // needed here?

		// setup bus1/bus2 image
		bus1ImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( bus1ImageInitInfo, inSourceInitInfoPtr );
		bus1Image = datatype<IImage>::create( GetKernelServices() );
		bus1Image->InitInstance( bufferImageInitInfo.get() );

		bus2ImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( bus1ImageInitInfo, inSourceInitInfoPtr );
		bus2Image = datatype<IImage>::create( GetKernelServices() );
		bus2Image->InitInstance( bufferImageInitInfo.get() );

		// setup black screen
		blackScreenInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		blackScreenInitInfo->SetColorModel( ecmBW );
		blackScreenInitInfo->SetPixelModel( epm8u );
		blackScreenInitInfo->SetWidth( _pInSource->GetWidth() ); 
		blackScreenInitInfo->SetHeight( _pInSource->GetHeight() );
		blackScreenInitInfo->_clear;
		blackScreen = datatype<IImage>::create( GetKernelServices() );
		blackScreen->InitInstance( blackScreenInitInfo.get() );

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CGhost::Start() throw()
{
    try
    {
		// start...
		minLowerThresholdValue = 0;
		maxUpperThresholdValue = 255;
		lowerValueParam = 18;
		upperValueParam = 127;
		gammaParam = 32;

		frameCounter = 0;
		delayTime = 3; // as a user param?

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CGhost::Execute() throw()
{
    try
    {
		// limit the frameCounter
		if (frameCounter <= delayTime) {
			frameCounter++;
		}
		// get parameters
		gammaParam = _pInGamma->GetValue();
		if (gammaParam < 1 || gammaParam > 254) {
			// invalid value
			gammaParam = 32;
		}

		lowerValueParam = _pParamThresholdLower->GetValue();
		if (lowerValueParam < 0 || lowerValueParam > 63) {
			// invalid value
			lowerValueParam = 18;
		}

		if (convertCM) {
			convertImagePtr->ConvertColorModel( _pInSource.get() );
			imageList->PushFront( convertImagePtr.get() );
			// set up bus 1
			bus1Image->CopyFrom( convertImagePtr.get() );
		}
		else {
			imageList->PushFront( _pInSource.get() );
			// set up bus 1
			bus1Image->CopyFrom(  _pInSource.get() );
		}

		if (frameCounter >= delayTime ) {
			// start copyfrom list
			bufferImage->CopyFrom( imageList->Back() );
			imageList->PopBack();
			bufferImage->CopyFrom( imageList->Back() );
		}
		
		// subtract delayed buffer
		bus1Image->SubtractAbsolute( bufferImage.get() );
		// threshold op
		bus1Image->ThresholdInt_LTGT( minLowerThresholdValue, lowerValueParam, maxUpperThresholdValue, upperValueParam );
		// bus1Image is now ready
		bus1Image->Max( bus2Image.get() );
		// set up bus 2
		bus2Image->CopyFrom( bus1Image.get() );
		bus2Image->SubtractInt( gammaParam );
		bus2Image->Max( blackScreen.get() );
		
		// final output mix image, boost for low levels
		_pOutFinalMix->CopyFrom( bus1Image.get() );
		// add
		_pOutFinalMix->Add( bus1Image.get() );
		_pOutFinalMix->Add( bus1Image.get() );

		_pOutFinalMix->SetCreationTime( _clockPtr->GetTime() );

    }
    catch(...)
    {
    }
	return true;
}

void CGhost::Stop() throw()
{
    try
    {
		// reset when patch stopped
		frameCounter = 0;
		imageList->Clear();
    }
    catch(...)
    {
    }
}

void CGhost::Done() throw()
{
    try
    {
		_pInSource = NULL;
		_pOutFinalMix = NULL;
		bufferImage = NULL;
		convertImagePtr = NULL;
		bus1Image = NULL;
		bus2Image = NULL;
		blackScreen = NULL;
		imageList = NULL;

    }
    catch(...)
    {
    }
}
