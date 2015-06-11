#include "StdAfx.h"
#include "DiffFilter.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_DiffFilter( 
	Eyw::block_class_registrant::block_id( "DiffFilter" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "DiffFilter" )
			.description( "Build 1.0.8.8 \nInput image threshold difference based filter.\n "
							"Utilising a Bang trigger, copy to buffer, IArith abs_diff, threshold op and median filter. Gain added at final output.\n"
							"This block requires a black and white image but will convert from color if required.\n" )
			.libraries( "OSL" )
			.bitmap( IDB_DELAYBW_BITMAP )
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
		.default_factory< CDiffFilter >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define PARAMETER_PINTRIGGER "pInTrigger"
#define PARAMETER_PINTHRESHOLDLOWER "pInThresholdLower"
#define PARAMETER_PINSIZEINT "pInSizeInt"
#define INPUT_PINSOURCE "pInSource"
#define OUTPUT_POUTFINALIMAGE "pOutFinalImage"


/////////////////////////////////
CDiffFilter::CDiffFilter( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource = NULL;
	_pOutFinalImage = NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_PINSOURCE, true );

}

CDiffFilter::~CDiffFilter()
{
}

void CDiffFilter::InitSignature()
{
	_pInTrigger= Eyw::Cast<Eyw::ITrigger*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_PINTRIGGER)
	                         .name("pInTrigger")
	                         .description("Trigger for snapshot of source image")
	                         .type<Eyw::ITrigger>()
	                         )->GetDatatype() );
	_pInThresholdLower= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_PINTHRESHOLDLOWER)
	                         .name("pInThresholdLower")
	                         .description("input integer for threshold lower setting")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(63, true)
	                         )->GetDatatype() );

	_pInSizeInt= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_PINSIZEINT)
	                         .name("pInSizeInt")
	                         .description("input size integer for filter mask, step 2, odd numbers only")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(1, true)
							 .max(11, true)
	                         )->GetDatatype() );
	SetInput(Eyw::pin::id(INPUT_PINSOURCE)
	    .name("pInSource")
	    .description("input source video")
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_POUTFINALIMAGE)
	    .name("pOutFinalImage")
	    .description("output final image")
	    .type<Eyw::IImage>()
	    );

	// default values
	_pInThresholdLower->SetValue( 28 );
	_pInSizeInt->SetValue( 3 );

}

void CDiffFilter::CheckSignature()
{
	_pInTrigger=get_parameter_datatype<Eyw::ITrigger>(PARAMETER_PINTRIGGER);
	_pInThresholdLower=get_parameter_datatype<Eyw::IInt>(PARAMETER_PINTHRESHOLDLOWER);
	_pInSizeInt=get_parameter_datatype<Eyw::IInt>(PARAMETER_PINSIZEINT);
	_signaturePtr->GetInputs()->FindItem( INPUT_PINSOURCE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_POUTFINALIMAGE );

}

void CDiffFilter::DoneSignature()
{
	_pInTrigger = NULL;
	_pInThresholdLower = NULL;
	_pInSizeInt = NULL;

}

// Actions
bool CDiffFilter::Init() throw()
{
    try
    {
		debugColorModel = 17; //unknown
		convertCM = false;

		_pInSource = get_input_datatype<Eyw::IImage>( INPUT_PINSOURCE );
		_pOutFinalImage = get_output_datatype<Eyw::IImage>( OUTPUT_POUTFINALIMAGE );
		
		inSourceInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inSourceInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );

		if( ( inSourceInitInfoPtr->GetColorModel() != ecmBW ) )
		{
			inSourceInitInfoPtr->SetColorModel( ecmBW );
			Notify_MessageString( "\nDiffFilter block: Source colorModel is not ecmBW, creating ecmBW convertor.\n" );
			convertImageInitInfo = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
			copy_datatype_init_info( convertImageInitInfo, inSourceInitInfoPtr );
			convertImagePtr = datatype<IImage>::create( GetKernelServices() );
			convertImagePtr->InitInstance( convertImageInitInfo.get() );
			
			convertCM = true;
		}
		else 
		{
			//
			Notify_MessageString( "\nDiffFilter block: Source colorModel is ecmBW.\n" );
			convertImageInitInfo = NULL;
			convertImagePtr = NULL;
			convertCM = false;
		}

		// set output to match input initInfo
		outFinalImageInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		copy_datatype_init_info( outFinalImageInitInfoPtr, inSourceInitInfoPtr );
		_pOutFinalImage->InitInstance( outFinalImageInitInfoPtr.get() );

		// setup temp image
		tempImageInitInfo = datatype_init_info<IImageInitInfo>::create( GetKernelServices() );
		copy_datatype_init_info( tempImageInitInfo, inSourceInitInfoPtr );
		tempImagePtr = datatype<IImage>::create( GetKernelServices() );
		tempImagePtr->InitInstance( tempImageInitInfo.get() );

		// setup snapshot image
		snapshotImageInitInfo = datatype_init_info<IImageInitInfo>::create( GetKernelServices() );
		copy_datatype_init_info( snapshotImageInitInfo, inSourceInitInfoPtr );
		snapshotImagePtr = datatype<IImage>::create( GetKernelServices() );
		snapshotImagePtr->InitInstance( snapshotImageInitInfo.get() );

		// remove
		//debugStuff();

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CDiffFilter::Start() throw()
{
    try
    {
		// default values
		sizeParam = 3;
		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;
		anchorSize._x = 1; 
		anchorSize._y = 1;

		minLowerThresholdValue = 0;
		maxUpperThresholdValue = 255;
		lowerValueParam = 28;
		upperValueParam = 127;

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CDiffFilter::Execute() throw()
{
    try
    {
		lowerValueParam = _pInThresholdLower->GetValue();
		if (lowerValueParam < 0 || lowerValueParam > 63) {
			// invalid value
			lowerValueParam = 28;
		}

		sizeParam = _pInSizeInt->GetValue();
		if (sizeParam < 1 || sizeParam > 11) {
			sizeParam = 3;
		}
		// sanity checks for sizeParam: odd numbers only, step 2
		if (sizeParam % 2 == 0) {
			// not an odd number use default
			Notify_MessageString( "\nDiffFilter block.WARNING: sizeParam must be odd number.\n" );
			sizeParam = 3;
		}

		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;
	
		if (convertCM) {
			convertImagePtr->ConvertColorModel( _pInSource.get() );
			tempImagePtr->CopyFrom( convertImagePtr.get() );
		}
		else 
		{
			// source already in BW colorModel
			tempImagePtr->CopyFrom( _pInSource.get() );
		}

		// subtract snapshot
		tempImagePtr->SubtractAbsolute( snapshotImagePtr.get() );
		// threshold op
		tempImagePtr->ThresholdInt_LTGT( minLowerThresholdValue, lowerValueParam, maxUpperThresholdValue, upperValueParam );
		// non-linear median filter
		tempImagePtr->FilterMedian( tempImagePtr.get(), maskSize, anchorSize );

		// final output
		_pOutFinalImage->CopyFrom( tempImagePtr.get() );
		//gain
		_pOutFinalImage->Add( tempImagePtr.get() );
		_pOutFinalImage->SetCreationTime( _clockPtr->GetTime() );
    }
    catch(...)
    {
    }
	return true;
}

void CDiffFilter::Stop() throw()
{
    try
    {
    }
    catch(...)
    {
    }
}

void CDiffFilter::Done() throw()
{
    try
    {
		_pInSource = NULL;
		_pOutFinalImage = NULL;
		snapshotImagePtr = NULL;
		tempImagePtr = NULL;
		convertImagePtr = NULL;

    }
    catch(...)
    {
    }
}

void CDiffFilter::OnChangedParameter(const std::string &parameterId)
{
	if (parameterId == PARAMETER_PINTRIGGER) {
		// do something, take snapshot
		BOOST_ASSERT(IsRunTime()); //only allow trigger during patch run, not IsDesignTime

		if (convertCM) {
			convertImagePtr->ConvertColorModel( _pInSource.get() );
			tempImagePtr->CopyFrom( convertImagePtr.get() );
		}
		else 
		{
			// source already in BW colorModel
			tempImagePtr->CopyFrom( _pInSource.get() );
		}
		snapshotImagePtr->CopyFromBuffer( tempImagePtr->GetBuffer(), tempImagePtr->GetStepSize() );
		
		// set sizeParam at snapshot trigger
		sizeParam = _pInSizeInt->GetValue();
		// sanity checks for sizeParam: odd numbers only, step 2
		if (sizeParam % 2 == 0) {
			// not an odd number use default
			sizeParam = 3;
		}

		if (sizeParam > 11) {
			// reached max value, force
			sizeParam = 11;
		}

		maskSize._cx = sizeParam;
		maskSize._cy = sizeParam;
	}
	else 
	{
		Eyw::CBlockImpl::OnChangedParameter(parameterId);
	}
}

void CDiffFilter::debugStuff() {
	//
	Notify_MessageString( "\nDiffFilter block: defaultCM: %d\n", debugColorModel );

	debugColorModel = inSourceInitInfoPtr->GetColorModel();
	Notify_MessageString( "\nDiffFilter block: inCM: %d\n", debugColorModel );

	debugColorModel = outFinalImageInitInfoPtr->GetColorModel();
	Notify_MessageString( "\nDiffFilter block: outCM: %d\n", debugColorModel );

	debugColorModel = tempImageInitInfo->GetColorModel();
	Notify_MessageString( "\nDiffFilter block: tempCM: %d\n", debugColorModel );

	debugColorModel = snapshotImageInitInfo->GetColorModel();
	Notify_MessageString( "\nDiffFilter block: snapshotCM: %d\n", debugColorModel );

}
