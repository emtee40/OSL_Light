#include "StdAfx.h"
#include "Flow.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_Flow( 
	Eyw::block_class_registrant::block_id( "Flow" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "Flow" )
			.description( "Build 1.0.1.6\nFlowing image stream.\n" 
							"Utilising image list, pixelwise comparator, image bus, color channel compose, color fraction.\n"
							"This block requires a black and white image but will convert from color if required.\n" )
			.libraries( "OSL" )
			.bitmap( IDB_FLOW_BITMAP )
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
		.default_factory< CFlow >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define PARAMETER_SIZE "size"
#define PARAMETER_FRACTIONB "fractionB"
#define PARAMETER_FRACTIONG "fractionG"
#define PARAMETER_FRACTIONR "fractionR"
#define INPUT_SOURCE "source"
#define OUTPUT_FINALOUT "finalOut"


/////////////////////////////////
CFlow::CFlow( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource=NULL;
	_pOutFinalOut=NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_SOURCE, true );

}

CFlow::~CFlow()
{
}

void CFlow::InitSignature()
{
	_pParamSize= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_SIZE)
	                         .name("size")
	                         .description("filter size")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(1, true)
							 .max(11, true)
	                         )->GetDatatype() );
	_pParamFractionB= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_FRACTIONB)
	                         .name("fractionB")
	                         .description("fraction of Blue")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );
	_pParamFractionG= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_FRACTIONG)
	                         .name("fractionG")
	                         .description("fraction of Green")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );
	_pParamFractionR= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_FRACTIONR)
	                         .name("fractionR")
	                         .description("fraction of Red")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );
	SetInput(Eyw::pin::id(INPUT_SOURCE)
	    .name("source")
	    .description("input source")
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_FINALOUT)
	    .name("finalOut")
	    .description("final BGR output")
	    .type<Eyw::IImage>()
	    );

	_pParamSize->SetValue( 3 );
	_pParamFractionB->SetValue( 0.5 );
	_pParamFractionG->SetValue( 0.5 );
	_pParamFractionR->SetValue( 0.5 );
}

void CFlow::CheckSignature()
{
	_pParamSize=get_parameter_datatype<Eyw::IInt>(PARAMETER_SIZE);
	_pParamFractionB=get_parameter_datatype<Eyw::IDouble>(PARAMETER_FRACTIONB);
	_pParamFractionG=get_parameter_datatype<Eyw::IDouble>(PARAMETER_FRACTIONG);
	_pParamFractionR=get_parameter_datatype<Eyw::IDouble>(PARAMETER_FRACTIONR);
	_signaturePtr->GetInputs()->FindItem( INPUT_SOURCE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_FINALOUT );

}

void CFlow::DoneSignature()
{
	_pParamSize=NULL;
	_pParamFractionB=NULL;
	_pParamFractionG=NULL;
	_pParamFractionR=NULL;

}

// Actions
bool CFlow::Init() throw()
{
    try
    {

		convertCM = false;

		_pInSource = get_input_datatype<Eyw::IImage>( INPUT_SOURCE );
		_pOutFinalOut = get_output_datatype<Eyw::IImage>( OUTPUT_FINALOUT );
		
		inSourceInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inSourceInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );

		if( ( _pInSource->GetColorModel() != ecmBW ) )
		{
			Notify_MessageString( "\nFlow block:: Source colorModel is not ecmBW, creating ecmBW convertor.\n" );
			convertImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
			copy_datatype_init_info( convertImageInitInfo, inSourceInitInfoPtr );
			convertImagePtr = datatype<IImage>::create( GetKernelServices() );
			convertImageInitInfo->SetColorModel( ecmBW );
			convertImagePtr->InitInstance( convertImageInitInfo.get() );
			convertCM = true;
		}
		else 
		{
			//
			Notify_MessageString( "\nFlow block: Source colorModel is ecmBW.\n" );
			convertImageInitInfo = NULL;
			convertImagePtr = NULL;
			convertCM = false;
		}

		// set output to match input initInfo and ecmBGR
		outFinalOutInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( outFinalOutInitInfoPtr, inSourceInitInfoPtr );
		outFinalOutInitInfoPtr->SetColorModel( ecmBGR );
		_pOutFinalOut->InitInstance( outFinalOutInitInfoPtr.get() );

		// setup temp image
		tempImageInitInfo = datatype_init_info<IImageInitInfo>::create( GetKernelServices() );
		copy_datatype_init_info( tempImageInitInfo, inSourceInitInfoPtr );
		tempImagePtr = datatype<IImage>::create( GetKernelServices() );
		tempImagePtr->InitInstance( tempImageInitInfo.get() );

		// setup adjust image
		adjustImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( adjustImageInitInfo, inSourceInitInfoPtr );
		adjustImagePtr = datatype<IImage>::create( GetKernelServices() );
		adjustImagePtr->InitInstance( adjustImageInitInfo.get() );

		// setup channelBGR images
		channelBGRInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( channelBGRInitInfo, inSourceInitInfoPtr );
		channelB = datatype<IImage>::create( GetKernelServices() );
		channelG = datatype<IImage>::create( GetKernelServices() );
		channelR = datatype<IImage>::create( GetKernelServices() );
		channelBGRInitInfo->SetColorModel( ecmBW );
		channelB->InitInstance( channelBGRInitInfo.get() );
		channelG->InitInstance( channelBGRInitInfo.get() );
		channelR->InitInstance( channelBGRInitInfo.get() );

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

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CFlow::Start() throw()
{
    try
    {
		frameCounter = 0;
		delayTime = 2;
		bufferCount = 4;

		paramB = 0.5;
		paramG = 0.5;
		paramR = 0.5;

		sizeParam = 3;
		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;
		anchorSize._x = 1; 
		anchorSize._y = 1;

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CFlow::Execute() throw()
{
    try
    {
		if (convertCM) {
			convertImagePtr->ConvertColorModel( _pInSource.get() );
			tempImagePtr->CopyFrom( convertImagePtr.get() );
		}
		else 
		{
			// source already in BW colorModel
			tempImagePtr->CopyFrom( _pInSource.get() );
		}

		//limit the counter
		if (frameCounter <= bufferCount) {
			frameCounter++;
		}

		// start sending to list
		imageList->PushFront( tempImagePtr.get() );

		if (frameCounter >= delayTime ) {
			// start copyfrom list
			adjustImagePtr->CopyFrom( imageList->Back() );
			imageList->PopBack();
		}

		sizeParam = _pParamSize->GetValue();
		if (sizeParam < 3 || sizeParam > 11) {
			sizeParam = 3;
		}
		if (sizeParam >= 5) {
			Notify_MessageString( "Flow block: caution FPS reduction at size > 5\n" );
		}
		if (sizeParam % 2 == 0) {
			// not an odd number use default
			Notify_MessageString( "\nFlow block.WARNING: sizeParam must be odd number.\n" );
			sizeParam = 3;
		}

		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;

		// comparator
		tempImagePtr->PixelwiseDifferentFrom( adjustImagePtr.get() );
		
		bufferList->PushFront( tempImagePtr.get() );

		// compose bgr from 3 greyscale images
		//0, 1, 2 = b, g, r
		paramB = _pParamFractionB->GetValue();
		if (paramB < 0.0 || paramB > 1.0) {
			// invalid value
			paramB = 0.5;
		}
		paramG = _pParamFractionG->GetValue();
		if (paramG < 0.0 || paramG > 1.0) {
			// invalid value
			paramG = 0.5;
		}
		paramR = _pParamFractionR->GetValue();
		if (paramR < 0.0 || paramR > 1.0) {
			// invalid value
			paramR = 0.5;
		}
	
		// start the channel copy
		channelB->CopyFrom( bufferList->Front() );

		if (bufferList->Size() >= 3) {
			channelG->CopyFrom( bufferList->Back() );
			bufferList->PopBack();
			channelR->CopyFrom( bufferList->Back() );
		}
		// set color fractions
		channelB->SubtractDouble( paramB ); // blue
		channelG->SubtractDouble( paramG ); // green
		channelR->SubtractDouble( paramR ); // red
		// combine channelBGR to output
		_pOutFinalOut->CopySingleChannel( channelB.get(), 0 );
		_pOutFinalOut->CopySingleChannel( channelG.get(), 1 );
		_pOutFinalOut->CopySingleChannel( channelR.get(), 2 );

		_pOutFinalOut->FilterMedian( _pOutFinalOut.get(), maskSize, anchorSize );
		
		

		//gain
		_pOutFinalOut->Add( _pOutFinalOut.get() );

		_pOutFinalOut->SetCreationTime( _clockPtr->GetTime() );
    }
    catch(...)
    {
    }
	return true;
}

void CFlow::Stop() throw()
{
    try
    {
		frameCounter = 0;
		imageList->Clear();
		bufferList->Clear();
    }
    catch(...)
    {
    }
}

void CFlow::Done() throw()
{
    try
    {
		_pInSource = NULL;
		_pOutFinalOut = NULL;
		convertImagePtr = NULL;
		bufferList = NULL;
		imageList = NULL;
		channelB = NULL;
		channelG = NULL;
		channelR = NULL;
    }
    catch(...)
    {
    }
}
