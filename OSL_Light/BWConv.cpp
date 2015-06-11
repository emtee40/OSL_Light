#include "StdAfx.h"
#include "BWConv.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_BWConv( 
	Eyw::block_class_registrant::block_id( "BWConv" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "BWConv" )
			.description( "Build 1.0.0.2\nInput color image converted to Black and White output.\n" 
							"Direct conversion from ecmBGR to ecmBW, will bypass if source already BW." )
			.libraries( "OSL" )
			.bitmap( IDB_BWCONV_BITMAP )
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
		.default_factory< CBWConv >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define INPUT_INSOURCE "inSource"
#define OUTPUT_OUTCONVERT "outConvert"


/////////////////////////////////
CBWConv::CBWConv( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource=NULL;
	_pOutConvert=NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_INSOURCE, true );

}

CBWConv::~CBWConv()
{
}

void CBWConv::InitSignature()
{
	SetInput(Eyw::pin::id(INPUT_INSOURCE)
	    .name("inSource")
	    .description("input color video")
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_OUTCONVERT)
	    .name("outConvert")
	    .description("output black and white video")
	    .type<Eyw::IImage>()
	    );

}

void CBWConv::CheckSignature()
{
	_signaturePtr->GetInputs()->FindItem( INPUT_INSOURCE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_OUTCONVERT );

}

void CBWConv::DoneSignature()
{

}

// Actions
bool CBWConv::Init() throw()
{
    try
    {
		
		convertCM = true;

		_pInSource = get_input_datatype<Eyw::IImage>( INPUT_INSOURCE );
		inSourceInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inSourceInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );
		
		// checks
		if( ( _pInSource->GetColorModel() == ecmBW ) )
		{
			Notify_MessageString( "\nDelayColor block: Source colorModel is already ecmBW, no conversion necessary.\n" );
			convertCM = false;
		}


		_pOutConvert = get_output_datatype<Eyw::IImage>( OUTPUT_OUTCONVERT );
		outConvertInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		// copy all relevant fields from source to output
		outConvertInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );
		// except, needs to be ecmBW output...
		outConvertInitInfoPtr->SetColorModel( ecmBW );
		_pOutConvert->InitInstance( outConvertInitInfoPtr.get() );

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CBWConv::Start() throw()
{
    try
    {
    	// TODO: add your logic
    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CBWConv::Execute() throw()
{
    try
    {
    	if (convertCM) {
			// convert colour video to BW
			_pOutConvert->ConvertColorModel( _pInSource.get() );
			Notify_MessageString( "\nDelayColor block: convertCM true.\n" );
		}
		else {
			// bypass conversion
			_pOutConvert->CopyFrom( _pInSource.get() );
			Notify_MessageString( "\nDelayColor block: convertCM false.\n" );
		}
		_pOutConvert->SetCreationTime( _clockPtr->GetTime() );
    }
    catch(...)
    {
    }
	return true;
}

void CBWConv::Stop() throw()
{
    try
    {
    }
    catch(...)
    {
    }
}

void CBWConv::Done() throw()
{
    try
    {
		_pInSource = NULL;
		_pOutConvert = NULL;

    }
    catch(...)
    {
    }
}
