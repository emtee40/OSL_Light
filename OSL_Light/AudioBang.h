#pragma once

class CAudioBang : public Eyw::CBlockImpl
{
public:
	CAudioBang( const Eyw::OBJECT_CREATIONCTX* ctxPtr );
	~CAudioBang();

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

	virtual void OnChangedParameter(const std::string &parameterId);

private:
	Eyw::double_ptr _pParamInThreshold;
	Eyw::trigger_ptr _pOutTrigger;

	Eyw::int_ptr _pParamRangeGate;

	Eyw::bool_ptr _pBoolTrigger;

	Eyw::audio_buffer_ptr _pInAudio;
	Eyw::audio_buffer_init_info_ptr inAudioInitInfo;

	Eyw::audio_buffer_ptr _pOutAudio;
	Eyw::audio_buffer_init_info_ptr outAudioInitInfo;

	//Eyw::device_ptr soundDevice;
	//Eyw::datatype_init_info_ptr soundDeviceInitInfo;
	//Eyw::IDSoundInputDevice...
	//Eyw::device_traits<IDSoundInputDevice> soundDeviceTraits;
	//Eyw::CDSoundInputDevicePtr soundDevicePtr; // deprecated...

	double paramThreshold;
	double inAudioSamplingRate;
	long inAudioNumSamples;	
	long inAudioNumChannels;

	bool clockTrigger;
	Eyw::TIME timer;

	int rangeCounter;
	int rangeGate;
	int rangeTimer;

};
