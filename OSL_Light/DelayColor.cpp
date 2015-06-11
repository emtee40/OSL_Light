#include "StdAfx.h"
#include "DelayColor.h"
#include "Signature.h"
#include "resource.h"

using namespace Eyw;

////////////////////////////////////////////////////////////////////////////////////
// Signature
Eyw::block_class_registrant g_DelayColor( 
	Eyw::block_class_registrant::block_id( "DelayColor" )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "DelayColor" )
			.description( "DelayColor build 1.0.1.18\nBlack and White source image delayed and combined into color image.\n"
							"Utilises image list buffers, internal image bus system, bypass checks.\n"
							"This block requires an ecmBW image source and will output ecmBGR. Delay value is number of frames buffered, 0 = no delay.\n")
			.libraries( "OSL" )
			.bitmap( IDB_DELAYCOLOR_BITMAP )
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
		.default_factory< CDelayColor >()
	);

//////////////////////////////////////////////////////////////////////
// Identifiers
#define PARAMETER_INDELAY1 "inDelay1"
#define PARAMETER_INDELAY2 "inDelay2"
#define PARAMETER_INFRACTIONB "inFractionB"
#define PARAMETER_INFRACTIONG "inFractionG"
#define PARAMETER_INFRACTIONR "inFractionR"
#define INPUT_INSOURCE "inSource"
#define OUTPUT_OUTFINAL "outFinal"


/////////////////////////////////
CDelayColor::CDelayColor( const Eyw::OBJECT_CREATIONCTX* ctxPtr )
:	Eyw::CBlockImpl( ctxPtr )
{
	_pInSource=NULL;
	_pOutFinal=NULL;

	_schedulingInfoPtr->SetActivationEventBased( true );
	_schedulingInfoPtr->GetEventBasedActivationInfo()->SetActivationOnInputChanged( INPUT_INSOURCE, true );

}

CDelayColor::~CDelayColor()
{
}

void CDelayColor::InitSignature()
{
	_pParamInDelay1= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_INDELAY1)
	                         .name("inDelay1")
	                         .description("number of delayed frames")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(50, true)
	                         )->GetDatatype() );

	_pParamInDelay2= Eyw::Cast<Eyw::IInt*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_INDELAY2)
	                         .name("inDelay2")
	                         .description("number of delayed frames")
	                         .type<Eyw::IInt>()
							 .set_int_domain(true)
							 .min(0, true)
							 .max(50, true)
	                         )->GetDatatype() );

	_pParamInFractionB= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_INFRACTIONB)
	                         .name("inFractionB")
	                         .description("percentage of blue color")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );
	_pParamInFractionG= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_INFRACTIONG)
	                         .name("inFractionG")
	                         .description("percent of green color")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );
	_pParamInFractionR= Eyw::Cast<Eyw::IDouble*>(
	                     SetParameter(Eyw::pin::id(PARAMETER_INFRACTIONR)
	                         .name("inFractionR")
	                         .description("percent of red color")
	                         .type<Eyw::IDouble>()
							 .set_double_domain(true)
							 .min(0.0, true)
							 .max(1.0, true)
	                         )->GetDatatype() );

	SetInput(Eyw::pin::id(INPUT_INSOURCE)
	    .name("inSource")
	    .description("black and white source")
	    .type<Eyw::IImage>()
	    );
	SetOutput(Eyw::pin::id(OUTPUT_OUTFINAL)
	    .name("outFinal")
	    .description("color image output")
	    .type<Eyw::IImage>()
	    );

	// default values
	_pParamInDelay1->SetValue( 25 );
	_pParamInDelay2->SetValue( 12 );
	_pParamInFractionB->SetValue( 0.5 );
	_pParamInFractionG->SetValue( 0.5 );
	_pParamInFractionR->SetValue( 0.5 );
}

void CDelayColor::CheckSignature()
{
	_pParamInDelay1=get_parameter_datatype<Eyw::IInt>(PARAMETER_INDELAY1);
	_pParamInDelay2=get_parameter_datatype<Eyw::IInt>(PARAMETER_INDELAY2);
	_pParamInFractionB=get_parameter_datatype<Eyw::IDouble>(PARAMETER_INFRACTIONB);
	_pParamInFractionG=get_parameter_datatype<Eyw::IDouble>(PARAMETER_INFRACTIONG);
	_pParamInFractionR=get_parameter_datatype<Eyw::IDouble>(PARAMETER_INFRACTIONR);
	_signaturePtr->GetInputs()->FindItem( INPUT_INSOURCE );
	_signaturePtr->GetOutputs()->FindItem( OUTPUT_OUTFINAL );

}

void CDelayColor::DoneSignature()
{
	_pParamInDelay1=NULL;
	_pParamInDelay2=NULL;
	_pParamInFractionB=NULL;
	_pParamInFractionG=NULL;
	_pParamInFractionR=NULL;

}

// Actions
bool CDelayColor::Init() throw()
{
    try
    {

		_pInSource = get_input_datatype<Eyw::IImage>( INPUT_INSOURCE );
		inSourceInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		inSourceInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );
		if( ( _pInSource->GetColorModel() != ecmBW ) )
		{
			Notify_MessageString( "\nDelayColor block. ERROR: Source colorModel is not ecmBW.\n" );
		}

		_pOutFinal = get_output_datatype<Eyw::IImage>( OUTPUT_OUTFINAL );
		outFinalInitInfoPtr = Eyw::datatype_init_info<Eyw::IImageInitInfo>::create( _kernelServicesPtr );
		outFinalInitInfoPtr->CopyFrom( _pInSource->GetInitInfo() );
		// needs to be ecmBGR output...
		outFinalInitInfoPtr->SetColorModel( ecmBGR );
		_pOutFinal->InitInstance( outFinalInitInfoPtr.get() );

		// init the buffer lists
		bufferListInitInfo = Eyw::datatype_init_info<Eyw::IListInitInfo>::create(_kernelServicesPtr);
		bufferListInitInfo->SetCatalogID( Eyw::datatype_traits<Eyw::IImage>::get_catalog_id() );
		bufferListInitInfo->SetClassID( Eyw::datatype_traits<Eyw::IImage>::get_class_id() );

		buffer1List = datatype<Eyw::IList>::create( GetKernelServices() );
		buffer1List->InitInstance(bufferListInitInfo.get());
		buffer1List->Resize(50); // buffer window size/length
		buffer1List->Clear();

		buffer2List = datatype<Eyw::IList>::create( GetKernelServices() );
		buffer2List->InitInstance(bufferListInitInfo.get());
		buffer2List->Resize(50); // buffer window size/length
		buffer2List->Clear();

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

		// bufferList parameter values
		buffer1Count = _pParamInDelay1->GetValue();
		buffer2Count = _pParamInDelay2->GetValue();
		if (buffer1Count < 0 || buffer1Count > 50) {
			buffer1Count = 25;
		}
		if (buffer2Count < 0 || buffer2Count > 50) {
			buffer2Count = 12;
		}

    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CDelayColor::Start() throw()
{
    try
    {
		// set defaults
		frameCounter = 0;
		//buffer1Count = 25;
		//buffer2Count = 12;

		lastBuffer1Count = buffer1Count;
		lastBuffer2Count = buffer2Count;
		// set maxCounter to highest bufferCount value
		(buffer1Count >= buffer2Count) ? maxCounter = buffer1Count : maxCounter = buffer2Count;

		paramB = 0.5;
		paramG = 0.5;
		paramR = 0.5;
    	return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CDelayColor::Execute() throw()
{
    try
    {

		// set maxCounter to highest bufferCount value
		(buffer1Count >= buffer2Count) ? maxCounter = buffer1Count : maxCounter = buffer2Count;
		if (frameCounter <= maxCounter) {
			frameCounter++;
		}
		// send to lists
		if (buffer1Count > 0 ) {
			buffer1List->PushFront( _pInSource.get() );
		}
		if (buffer2Count > 0 ) {
			buffer2List->PushFront( _pInSource.get() );
		}
		
		// checks
		if (buffer1Count < 0 || buffer1Count > 50) {
			buffer1Count = 25;
		}
		if (buffer2Count < 0 || buffer2Count > 50) {
			buffer2Count = 12;
		}

		if (buffer1Count == 0 ) {
			//bypass buffer
			channelB->CopyFrom( _pInSource.get() );
		}

		else if (frameCounter >= buffer1Count ) {
			// copy from list
			channelB->CopyFrom( buffer1List->Back() );
			buffer1List->PopBack();
		}
		// always copy green
		channelG->CopyFrom( _pInSource.get() );


		if (buffer2Count == 0 ) {
			//bypass buffer
			channelR->CopyFrom( _pInSource.get() );
		}

		else if (frameCounter >= buffer2Count ) {
			// copy from list
			channelR->CopyFrom( buffer2List->Back() );
			buffer2List->PopBack();
		}

		// compose bgr from 3 greyscale images
		//0, 1, 2 = b, g, r
		paramB = _pParamInFractionB->GetValue();
		if (paramB < 0.0 || paramB > 1.0) {
			// invalid value
			paramB = 0.5;
		}
		paramG = _pParamInFractionG->GetValue();
		if (paramG < 0.0 || paramG > 1.0) {
			// invalid value
			paramG = 0.5;
		}
		paramR = _pParamInFractionR->GetValue();
		if (paramR < 0.0 || paramR > 1.0) {
			// invalid value
			paramR = 0.5;
		}

		// set color fractions
		channelB->SubtractDouble( paramB ); // blue
		channelG->SubtractDouble( paramG ); // green
		channelR->SubtractDouble( paramR ); // red
		// combine channelBGR to output
		_pOutFinal->CopySingleChannel( channelB.get(), 0 );
		_pOutFinal->CopySingleChannel( channelG.get(), 1 );
		_pOutFinal->CopySingleChannel( channelR.get(), 2 );

		//gain
		_pOutFinal->Add( _pOutFinal.get() );
		_pOutFinal->SetCreationTime( _clockPtr->GetTime() );

    }
    catch(...)
    {
    }
	return true;
}

void CDelayColor::OnChangedParameter(const std::string &parameterId)
{
	if (parameterId == PARAMETER_INDELAY1) {
		// do something, check values
		lastBuffer1Count = buffer1Count;

		buffer1Count = _pParamInDelay1->GetValue();
		if (buffer1Count < 0 || buffer1Count > 50) {
			buffer1Count = lastBuffer1Count;
		}
		// buffer window size/length
		/*
		if (buffer1Count == 0) {
			buffer1List->Resize(1); 
		}
		else {
			buffer1List->Resize(buffer1Count);
		}
		*/
		// set maxCounter to highest bufferCount value
		//(buffer1Count >= buffer2Count) ? maxCounter = buffer1Count : maxCounter = buffer2Count;
	}
	else if (parameterId == PARAMETER_INDELAY2) {
		// do something, check values
		lastBuffer2Count = buffer2Count;

		buffer2Count = _pParamInDelay2->GetValue();
		if (buffer2Count < 0 || buffer2Count > 50) {
			buffer2Count = lastBuffer2Count;
		}
		// buffer window size/length
		/*
		if (buffer2Count == 0) {
			buffer2List->Resize(1); 
		}
		else {
			buffer2List->Resize(buffer2Count);
		}
		*/
		// set maxCounter to highest bufferCount value
		//(buffer1Count >= buffer2Count) ? maxCounter = buffer1Count : maxCounter = buffer2Count;
	}
	else 
	{
		Eyw::CBlockImpl::OnChangedParameter(parameterId);
	}
}

void CDelayColor::Stop() throw()
{
    try
    {
		buffer1List->Clear();
		buffer2List->Clear();
		frameCounter = 0;
    }
    catch(...)
    {
    }
}

void CDelayColor::Done() throw()
{
    try
    {
		_pInSource = NULL;
		_pOutFinal = NULL;
		buffer1List = NULL;
		buffer2List = NULL;
		channelB = NULL;
		channelG = NULL;
		channelR = NULL;
    }
    catch(...)
    {
    }
}


