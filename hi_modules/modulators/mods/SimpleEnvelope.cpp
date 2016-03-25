
void SimpleEnvelope::restoreFromValueTree(const ValueTree &v)
{
	EnvelopeModulator::restoreFromValueTree(v);

	loadAttribute(Attack, "Attack");
	loadAttribute(Release, "Release");
	linearMode = v.getProperty("LinearMode", true); // default is on
}

ValueTree SimpleEnvelope::exportAsValueTree() const
{
	ValueTree v = EnvelopeModulator::exportAsValueTree();

	saveAttribute(Attack, "Attack");
	saveAttribute(Release, "Release");
	saveAttribute(LinearMode, "LinearMode");

	return v;
}

/*
  ==============================================================================

    SimpleEnvelope.cpp
    Created: 15 Jun 2014 2:08:15pm
    Author:  Chrisboy

  ==============================================================================
*/



SimpleEnvelope::SimpleEnvelope(MainController *mc, const String &id, int voiceAmount, Modulation::Mode m):
		EnvelopeModulator(mc, id, voiceAmount, m),
		Modulation(m),
		attack(getDefaultValue(Attack)),
		release(getDefaultValue(Release)),
		release_delta(-1.0f),
		linearMode(getDefaultValue(LinearMode) == 1.0f ? true : false),
		attackChain(nullptr)
{
	parameterNames.add("Attack");
	parameterNames.add("Release");

	editorStateIdentifiers.add("AttackChainShown");
	//editorStateIdentifiers.add("ReleaseChainShown");

	for(int i = 0; i < polyManager.getVoiceAmount(); i++) states.add(createSubclassedState(i));
	attackChain = new ModulatorChain(mc, "Attack Time Modulation", voiceAmount, ModulatorChain::GainMode, this);

	attackChain->setIsVoiceStartChain(true);

	attackChain->setColour(Colours::red);


};

SimpleEnvelope::~SimpleEnvelope()
{
	attackChain = nullptr;
};


void SimpleEnvelope::setInternalAttribute(int parameter_index, float newValue)
{
	switch (parameter_index)
	{
	case Attack:
		setAttackRate(newValue);
		
		break;
	case Release:
		release = newValue;
		setReleaseRate(newValue);
		break;
	case LinearMode:
		linearMode = newValue > 0.5;
		setAttackRate(attack);
		setReleaseRate(release);
		break;
	default:
		jassertfalse;
	}
}

float SimpleEnvelope::getDefaultValue(int parameterIndex) const
{
	switch (parameterIndex)
	{
	case Attack:
		return 5.0f;
	case Release:
		return 10.0f;
	case LinearMode:
		return 1.0f;
	default:
		jassertfalse;
		return -1;
	}
}

float SimpleEnvelope::getAttribute(int parameter_index) const
{
	switch (parameter_index)
	{
	case Attack:
		return attack;
	case Release:
		return release;
	case LinearMode:
		return linearMode ? 1.0f : 0.0f;
	default:
		jassertfalse;
		return -1;
	}
}

void SimpleEnvelope::startVoice(int voiceIndex)
{
	SimpleEnvelopeState *state = static_cast<SimpleEnvelopeState*>(states[voiceIndex]);

	if(state->current_state != SimpleEnvelopeState::IDLE)
	{
		reset(voiceIndex);
	}

	attackChain->startVoice(voiceIndex);

	if (linearMode)
	{
		state->attackDelta = this->calcCoefficient(attack * attackChain->getConstantVoiceValue(voiceIndex));
	}
	else
	{
		setAttackRate(attack * attackChain->getConstantVoiceValue(voiceIndex), voiceIndex);
	}

	
	state->current_state = SimpleEnvelopeState::ATTACK;
	debugMod(" (Voice " + String(voiceIndex) + ": IDLE->ATTACK");
}

void SimpleEnvelope::stopVoice(int voiceIndex)
{
	SimpleEnvelopeState *state = static_cast<SimpleEnvelopeState*>(states[voiceIndex]);
	//jassert(state->current_state == SimpleEnvelopeState::SUSTAIN || state->current_state == SimpleEnvelopeState::ATTACK);

	debugMod(" (Voice " + String(voiceIndex) + (state->current_state == SimpleEnvelopeState::SUSTAIN ? "SUSTAIN": "ATTACK") + "->RELEASE");
	state->current_state = SimpleEnvelopeState::RELEASE;

}

void SimpleEnvelope::reset(int voiceIndex)
{
	EnvelopeModulator::reset(voiceIndex);

	SimpleEnvelopeState *state = static_cast<SimpleEnvelopeState*>(states[voiceIndex]);

	state->current_state = SimpleEnvelopeState::IDLE;
	state->current_value = 0.0f;
}

bool SimpleEnvelope::isPlaying(int voiceIndex) const
{
	SimpleEnvelopeState *state = static_cast<SimpleEnvelopeState*>(states[voiceIndex]);
	return state->current_state != SimpleEnvelopeState::IDLE;
}

void SimpleEnvelope::calculateBlock(int startSample, int numSamples)
{
	const int voiceIndex = polyManager.getCurrentVoice();

	jassert(voiceIndex < states.size());

	state = static_cast<SimpleEnvelopeState*>(states[voiceIndex]);

	if (state->current_state == SimpleEnvelopeState::SUSTAIN)
	{
		FloatVectorOperations::fill(internalBuffer.getWritePointer(0, startSample), 1.0f, numSamples);
		setOutputValue(1.0f);
	}
	else if (state->current_state == SimpleEnvelopeState::IDLE)
	{
		FloatVectorOperations::fill(internalBuffer.getWritePointer(0, startSample), 0.0f, numSamples);
		setOutputValue(0.0f);
	}
	else
	{
		
		float *out = internalBuffer.getWritePointer(0, startSample);

		if (linearMode)
		{
			while (numSamples > 0)
			{
				*out++ = calculateNewValue();
				*out++ = calculateNewValue();
				*out++ = calculateNewValue();
				*out++ = calculateNewValue();

				numSamples -= 4;
			}
		}
		else
		{
			while (numSamples > 0)
			{
				*out++ = calculateNewExpValue();
				*out++ = calculateNewExpValue();
				*out++ = calculateNewExpValue();
				*out++ = calculateNewExpValue();

				numSamples -= 4;
			}
		}

		

		if (polyManager.getCurrentVoice() == polyManager.getLastStartedVoice()) setOutputValue(internalBuffer.getSample(0, 0));
	}
}

void SimpleEnvelope::handleMidiEvent(MidiMessage const &m)
{
	attackChain->handleMidiEvent(m);
};

void SimpleEnvelope::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	EnvelopeModulator::prepareToPlay(sampleRate, samplesPerBlock);
		

	setInternalAttribute(Attack, attack);
	setInternalAttribute(Release, release);
	if(attackChain != nullptr) attackChain->prepareToPlay(sampleRate, samplesPerBlock);
}

ProcessorEditorBody *SimpleEnvelope::createEditor(BetterProcessorEditor *parentEditor)
{
#if USE_BACKEND

	return new SimpleEnvelopeEditorBody(parentEditor);

#else

	jassertfalse;

	return nullptr;

#endif

};

float SimpleEnvelope::calcCoefficient(float time, float targetRatio/*=1.0f*/) const
{
	if (linearMode)
	{
		return 1.0f / ((time / 1000.0f) * (float)this->getSampleRate());
	}
	else
	{
		if (time == 0.0f) return 0.0f;

		const float factor = (float)getSampleRate() * 0.001f;

		time *= factor;
		return expf(-logf((1.0f + targetRatio) / targetRatio) / time);
	}
}

float SimpleEnvelope::calculateNewValue()
{
	switch (state->current_state)
	{
	case SimpleEnvelopeState::SUSTAIN: break;
	case SimpleEnvelopeState::IDLE: break;
	case SimpleEnvelopeState::ATTACK:
		state->current_value += state->attackDelta;
		if (state->current_value >= 1.0f)
		{
			state->current_value = 1.0f;
			state->current_state = SimpleEnvelopeState::SUSTAIN;
			//debugMod(" (voiceIndex = " + String(voiceIndex) + "): ATTACK->SUSTAIN");
		}
		break;
	case SimpleEnvelopeState::RELEASE:
		state->current_value -= release_delta;
		if (state->current_value <= 0.0f){
			state->current_value = 0.0f;
			state->current_state = SimpleEnvelopeState::IDLE;
			//debugMod(" (voiceIndex = " + String(voiceIndex) + "): RELEASE->IDLE");
		}
		break;
	default:					    jassertfalse; break;
	}

	return state->current_value;
}

float SimpleEnvelope::calculateNewExpValue()
{
	switch (state->current_state)
	{
	case SimpleEnvelopeState::SUSTAIN: break;
	case SimpleEnvelopeState::IDLE: break;
	case SimpleEnvelopeState::ATTACK:
		
		state->current_value = state->expAttackBase + state->current_value * state->expAttackCoef;

		if (state->current_value >= 1.0f)
		{
			state->current_value = 1.0f;
			state->current_state = SimpleEnvelopeState::SUSTAIN;
		}
		break;
	case SimpleEnvelopeState::RELEASE:
		state->current_value = expReleaseBase + state->current_value * expReleaseCoef;

		if (state->current_value <= 0.0001f){
			state->current_value = 0.0f;
			state->current_state = SimpleEnvelopeState::IDLE;
		}
		break;
	default:					    jassertfalse; break;
	}

	return state->current_value;
}



void SimpleEnvelope::setAttackRate(float rate, int voiceIndex/*=-1*/) {
	
	if (voiceIndex == -1)
	{
		attack = rate;

		if (linearMode)
		{
			expAttackCoef = 0.0f;
			expAttackBase = 1.0f;
		}
		else
		{
			const float targetRatioA = 0.3f;

			expAttackCoef = calcCoefficient(attack, targetRatioA);
			expAttackBase = (1.0f + targetRatioA) * (1.0f - expAttackCoef);
		}
	}
	else
	{
		SimpleEnvelopeState *state = static_cast<SimpleEnvelopeState*>(states[voiceIndex]);

		if (linearMode)
		{
			state->expAttackCoef = 0.0f;
			state->expAttackBase = 1.0f;
		}
		else
		{
			const float targetRatioA = 0.3f;

			state->expAttackCoef = calcCoefficient(rate, targetRatioA);
			state->expAttackBase = (1.0f + targetRatioA) * (1.0f - state->expAttackCoef);
		}
	}
}


void SimpleEnvelope::setReleaseRate(float rate)
{
	release = rate;

	if (linearMode)
	{
		expReleaseCoef = 0.0f;
		expReleaseBase = 1.0f;

		release_delta = calcCoefficient(release);
	}
	else
	{
		const float targetRatioR = 0.0001f;

		expReleaseCoef = calcCoefficient(release, targetRatioR);
		expReleaseBase = -targetRatioR * (1.0f - expReleaseCoef);
	}
}