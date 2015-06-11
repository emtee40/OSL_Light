#include "stdafx.h"
#include "resource.h"
#include "Signature.h"

using namespace Eyw;

static catalog_class_registrant gCatalog( 
	catalog_class_registrant::catalog_id( EYW_OSL_LIGHT_CATALOG_ID )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "OSL_light" )
			.description( "Light weight catalog for video projections" )
			.bitmap( IDB_OSL_LIGHT_CATALOG )
		.end_language()	
	);

static company_registrant gCompany( 
	company_registrant::company_id( EYW_OSL_LIGHT_COMPANY_ID )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "Object_State" )
			.description( "Motion capture and video projection group" )
		.end_language()	
	);

static licence_registrant gLicense( 
	licence_registrant::licence_id( EYW_OSL_LIGHT_LICENSE_ID )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "OSL_light Licence" )
			.description( "This catalog has the same license of EyesWeb XMI" )
		.end_language()	
	);

static author_registrant gAuthor( 
	author_registrant::author_id( EYW_OSL_LIGHT_CATALOG_AUTHOR_ID )
		.begin_language( EYW_LANGUAGE_US_ENGLISH )
			.name( "Kaputnik Go" )
			.description( "kaputnikgo@gmail.com" )
		.end_language()	
	);


