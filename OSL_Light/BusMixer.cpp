#include "StdAfx.h"
#include "BusMixer.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_BusMixer( 
	Eyw::block_class_registrant::block_id( "BusMixer" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "BusMixer" )
			.description( "Build 1.0.2.8 \nMix two video sources to single video output. \n"
							"Utilises subtract double and an internal IImage copy for mixing. Gain added at final output.\n" 
							"This block requires 2 video sources of matching color model, pixel model and size.")
			.libraries( "OSL" )
			.bitmap( IDB_BUSMIXER_BITMAP )
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
		.default_factory< CBusMixer >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define PARAMETER_PINMIXERVALUE "pInMixerValue"
#define INPUT_PINSOURCE1 "pInSource1"
#define INPUT_PINSOURCE2 "pInSource2"
#define OUTPUT_POUTFINALMIX "pOutFinalMix"


/////////////////////////////////
CBusMixer::CBusMixer( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource1 = NULL;
	_pInSource2 = NULL;
	_pOutFinalMix = NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_PINSOURCE1, true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_PINSOURCE2, true );

}

CBusMixer::~CBusMixer()
{
}

void CBusMixer::InitSignature()
{
	_pInMixerValue= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_PINMIXERVALUE)
	                         .name("pInMixerValue")
	                         .description("Mixer ratio between sources")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );
	SetInput(Eyw::pin::id(INPUT_PINSOURCE1)
	    .name("pInSource1")
	    .description("Video source input 1")
	    .type<Eyw::IImage>()
	    );
	SetInput(Eyw::pin::id(INPUT_PINSOURCE2)
	    .name("pInSource2")
	    .description("Video source input 2")
		.is_required(false)
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_POUTFINALMIX)
	    .name("pOutFinalMix")
	    .description("Mixed video output")
	    .type<Eyw::IImage>()
	    );

	// default value
	_pInMixerValue->SetValue( 0.5 );
}

void CBusMixer::CheckSignature()
{
	_pInMixerValue = get_parameter_datatype<Eyw::IDouble>(PARAMETER_PINMIXERVALUE);
	_signaturePtr->GetInputs()->FindItem( INPUT_PINSOURCE1 );
	_signaturePtr->GetInputs()->FindItem( INPUT_PINSOURCE2 );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_POUTFINALMIX );
}

void CBusMixer::DoneSignature()
{
	_pInMixerValue = NULL;

}

// Actions
bool CBusMixer::Init() throw()
{
    try
    {
		_pInSource1 = get_input_datatype<Eyw::IImage>( INPUT_PINSOURCE1 );
		_pInSource2 = get_input_datatype<Eyw::IImage>( INPUT_PINSOURCE2 );
		_pOutFinalMix = get_output_datatype<Eyw::IImage>( OUTPUT_POUTFINALMIX );

		inSource1InitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inSource1InitInfoPtr->CopyFrom( _pInSource1->GetInitInfo() );

		source2Present = (_pInSource2 != NULL);
		if ( source2Present ) 
		{
			inSource2InitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
			inSource2InitInfoPtr->CopyFrom( _pInSource2->GetInitInfo() );
		}
		else 
		{
			Notify_ErrorString( "\nBusMixer block ERROR: source 2 not present.\n" );
			return false;
		}

		// source1/2 checks
		if( ( inSource1InitInfoPtr->GetPixelModel() != inSource2InitInfoPtr->GetPixelModel() ) )
		{
			Notify_ErrorString( "\nBusMixer block ERROR: Source Pixel models do not match.\n" );
			return false;
		}

		if( ( inSource1InitInfoPtr->GetColorModel() != inSource2InitInfoPtr->GetColorModel() ) )
		{
			Notify_ErrorString( "\nBusMixer block ERROR: Source Color models do not match.\n" );
			return false;
		}

		if( ( inSource1InitInfoPtr->GetHeight() != inSource2InitInfoPtr->GetHeight() 
			|| inSource1InitInfoPtr->GetWidth() != inSource2InitInfoPtr->GetWidth() ) )
		{
			Notify_ErrorString( "\nBusMixer block ERROR: Source height/width does not match.\n" );
			return false;
		}

		outFinalMixInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		outFinalMixInitInfoPtr->CopyFrom( _pInSource1->GetInitInfo() );
		_pOutFinalMix->InitInstance( outFinalMixInitInfoPtr.get() );

		// setup temp image
		tempImageInitInfo = datatype_init_info<IImageInitInfo>::create( GetKernelServices() );
		copy_datatype_init_info( tempImageInitInfo, inSource1InitInfoPtr );
		tempImagePtr = datatype<IImage>::create( GetKernelServices() );
		tempImagePtr->InitInstance( tempImageInitInfo.get() );

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CBusMixer::Start() throw()
{
    try
    {
		defaultMixerValue = 0.5;
		paramMixerValue = 0.5;
		paramMixerValueInvert = 0.5;
    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CBusMixer::Execute() throw()
{
    try
    {
		paramMixerValue = _pInMixerValue->GetValue();

		if ( source2Present ) {
			// check for useful limits of parameter value
			if ( paramMixerValue < 0.000000000000001 ) {
				paramMixerValue = 0.000000000000001;
			}
			if ( paramMixerValue >= 0.999999999999999 ) {
				paramMixerValue = 0.999999999999999;
			}
			paramMixerValueInvert = 1.0 - paramMixerValue;

			tempImagePtr->CopyFrom( _pInSource1.get() );
			// mixing
			tempImagePtr->MultiplyDouble( paramMixerValue );
			// invert
			_pInSource2->MultiplyDouble( paramMixerValueInvert );
			// combine result
			tempImagePtr->Add( _pInSource2.get() );
			// copy to output
			_pOutFinalMix->CopyFrom( tempImagePtr.get() );
			_pOutFinalMix->SetCreationTime( _clockPtr->GetTime() );
		}
		else 
		{
			// has only one input, send directly to output
			Notify_MessageString( "BusMixer block ERROR: source 2 not present.\n" );
			_pOutFinalMix->CopyFrom( _pInSource1.get() );
			_pOutFinalMix->SetCreationTime( _clockPtr->GetTime() );
		}
    }
    catch(...)
    {
    }
	return true;
}

void CBusMixer::Stop() throw()
{
    try
    {
    }
    catch(...)
    {
    }
}

void CBusMixer::Done() throw()
{
    try
    {
		_pInSource1 = NULL;
		_pInSource2 = NULL;
		_pOutFinalMix = NULL;
		tempImagePtr = NULL;
    }
    catch(...)
    {
    }
}
