#pragma once

class CBWConv : public Eyw::CBlockImpl
{
public:
	CBWConv( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CBWConv();

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

private:
	Eyw::image_ptr _pInSource;
	Eyw::image_ptr _pOutConvert;

	Eyw::image_init_info_ptr inSourceInitInfoPtr;
	Eyw::image_init_info_ptr outConvertInitInfoPtr;

	bool convertCM;

};
