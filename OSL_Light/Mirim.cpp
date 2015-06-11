#include "StdAfx.h"
#include "Mirim.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_Mirim( 
	Eyw::block_class_registrant::block_id( "Mirim" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "Mirim" )
			.description( "Build 1.0.0.1\nMirror video at vertical axis and combine.\n"
							"Accepts any video stream, mirrors in the middle vertically \n"
							"and then combines the two to create a mandala type output image." )
			.libraries( "OSL" )
			.bitmap( IDB_MIRIM_BITMAP )
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
		.default_factory< CMirim >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define INPUT_INSOURCE "inSource"
#define OUTPUT_OUTFINAL "outFinal"


/////////////////////////////////
CMirim::CMirim( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource=NULL;
	_pOutFinal=NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_INSOURCE, true );

}

CMirim::~CMirim()
{
}

void CMirim::InitSignature()
{
	SetInput(Eyw::pin::id(INPUT_INSOURCE)
	    .name("inSource")
	    .description("video source")
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_OUTFINAL)
	    .name("outFinal")
	    .description("mirrored output")
	    .type<Eyw::IImage>()
	    );

}

void CMirim::CheckSignature()
{
	_signaturePtr->GetInputs()->FindItem( INPUT_INSOURCE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_OUTFINAL );

}

void CMirim::DoneSignature()
{

}

// Actions
bool CMirim::Init() throw()
{
    try
    {

		_pInSource = get_input_datatype<Eyw::IImage>( INPUT_INSOURCE );
		inputImageInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inputImageInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );

		_pOutFinal = get_output_datatype<Eyw::IImage>( OUTPUT_OUTFINAL );
		outputImageInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		outputImageInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );
		_pOutFinal->InitInstance( outputImageInitInfoPtr.get() );

		// setup temp image
		tempImageInitInfo = datatype_init_info<IImageInitInfo>::create( GetKernelServices() );
		copy_datatype_init_info( tempImageInitInfo, inputImageInitInfoPtr );
		tempImagePtr = datatype<IImage>::create( GetKernelServices() );
		tempImagePtr->InitInstance( tempImageInitInfo.get() );

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CMirim::Start() throw()
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

bool CMirim::Execute() throw()
{
    try
    {
    	// copy orig first
		tempImagePtr->CopyFrom( _pInSource.get() );

		// add a bool switch here? off/on?
		tempImagePtr->Mirror( eVerticalAxis );
		// mixing
		tempImagePtr->MultiplyDouble( 0.5 );
		// invert
		_pInSource->MultiplyDouble( 0.5 );
		// combine
		tempImagePtr->Add(_pInSource.get() );
		// 
		_pOutFinal->CopyFrom(tempImagePtr.get() );
		// gain
		_pOutFinal->Add(tempImagePtr.get() );
		_pOutFinal->SetCreationTime( _clockPtr->GetTime() );
    }
    catch(...)
    {
    }
	return true;
}

void CMirim::Stop() throw()
{
    try
    {
    }
    catch(...)
    {
    }
}

void CMirim::Done() throw()
{
    try
    {
		_pInSource = NULL;
		_pOutFinal = NULL;
		tempImagePtr = NULL;

    }
    catch(...)
    {
    }
}
