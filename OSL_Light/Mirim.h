#pragma once

class CMirim : public Eyw::CBlockImpl
{
public:
	CMirim( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CMirim();

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
	Eyw::image_ptr _pOutFinal;
	Eyw::image_init_info_ptr inputImageInitInfoPtr;
	Eyw::image_init_info_ptr outputImageInitInfoPtr;

	Eyw::image_ptr tempImagePtr;
	Eyw::image_init_info_ptr tempImageInitInfo;

};
