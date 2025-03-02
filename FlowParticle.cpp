// Copyright 2017-2021 Crytek GmbH / Crytek Group. All rights reserved.

/**********************************************************************************
**                             HPi Source File                                   **
**    Copyright (C) 2020-2025 HPiStudio. All rights Reserved.                    **
** ********************************************************************************
**                                                                               **
** Description:                                                                  **
**              simple followGraph Spawn Particle node                           **
**                                                                               **
** Created in sat 1403/12/12 11:33 PM By Hosein Pirani                           **
**                                                                               **
** Modified In sun 1403/12/13 16:15 PM To 12:43 AM by me.                        **
** :       detected bugs were fixed                                              **
** TODO:                                                                         **
** TODO:                                                                         **
** ..                                                                            **
** ...                                                                           **
** And CODE                                                                      **
** ..... More Code                                                               **
** ........ Code                                                                 **
** ...........  #_#                                                              **
** ...............                                                               **
*********************************************************************************/

#include "StdAfx.h"
#include <CryFlowGraph/IFlowBaseNode.h>
#include <CryParticleSystem/ParticleParams.h>


class CFlowParticleClass : public CFlowBaseNode<eNCT_Instanced>
{
public:
	IParticleEmitter* GetEmitter(SActivationInfo* pActInfo) const
	{
		if (!pActInfo->pEntity)
			return nullptr;
		return pActInfo->pEntity->GetParticleEmitter(0);
	}

private:
};
//-------------------------------------------------------------------------------------------------------------------------
class CFlowNode_ParticleAttributeGet : public CFlowParticleClass
{
public:
	enum class EInputs
	{
		Activate,
		Name,
	};

public:
	CFlowNode_ParticleAttributeGet(SActivationInfo* pActInfo)
	{
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ParticleAttributeGet(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inConfig[] =
		{
			InputPortConfig<SFlowSystemVoid>("Get",_HELP("")),
			InputPortConfig<string>("Attribute", _HELP(""), 0, _UICONFIG("ref_entity=entityId")),
			{ 0 }
		};

		static const SOutputPortConfig outConfig[] =
		{
			OutputPortConfig_AnyType("Value", _HELP("")),
			OutputPortConfig_AnyType("Error", _HELP("")),
			{ 0 }
		};

		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inConfig;
		config.pOutputPorts = outConfig;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (event != eFE_Activate)
			return;

		IParticleEmitter* pEmitter = GetEmitter(pActInfo);
		if (!pEmitter)
			return;

		const bool triggerPort = IsPortActive(pActInfo, int(EInputs::Activate));
		if (!triggerPort)
			return;

		const string inputPropertyName = GetPortString(pActInfo, int(EInputs::Name));
		IParticleAttributes& attributes = pEmitter->GetAttributes();
		auto attributeId = attributes.FindAttributeIdByName(inputPropertyName.c_str());
		if (attributeId == -1)
		{
			GameWarning(
				"[flow] CFlowNode_ParticleAttributeGet: Cannot resolve attribute '%s' in entity '%s'",
				inputPropertyName.c_str(), pActInfo->pEntity->GetName());
			ActivateOutput(pActInfo, 1, 0);
			return;
		}

		switch (attributes.GetAttributeType(attributeId))
		{
		case IParticleAttributes::ET_Integer:
			ActivateOutput(pActInfo, 0, attributes.GetAsInteger(attributeId, 0));
			break;
		case IParticleAttributes::ET_Float:
			ActivateOutput(pActInfo, 0, attributes.GetAsFloat(attributeId, 0.0f));
			break;
		case IParticleAttributes::ET_Boolean:
			ActivateOutput(pActInfo, 0, attributes.GetAsBoolean(attributeId, false));
			break;
		case IParticleAttributes::ET_Color:
			ActivateOutput(pActInfo, 0, attributes.GetAsColorF(attributeId, ColorF()).toVec3());
			break;
		}
	}

};
//-------------------------------------------------------------------------------------------------------------------------
class CFlowNode_ParticleAttributeSet : public CFlowParticleClass
{
public:
	enum class EInputs
	{
		Activate,
		Name,
		Value,
	};

public:
	CFlowNode_ParticleAttributeSet(SActivationInfo* pActInfo)
	{
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_ParticleAttributeSet(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
	
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inConfig[] =
		{
			InputPortConfig<SFlowSystemVoid>("Set",_HELP("")),
			InputPortConfig<string>("Attribute", _HELP(""), 0, _UICONFIG("ref_entity=entityId")),
			InputPortConfig<string>("Value",_HELP("")),
			{ 0 }
		};

		static const SOutputPortConfig outConfig[] =
		{
			OutputPortConfig_AnyType("Error", _HELP("")),
			{ 0 }
		};

		config.sDescription = _HELP("");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inConfig;
		config.pOutputPorts = outConfig;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		if (event != eFE_Activate)
			return;

		IParticleEmitter* pEmitter = GetEmitter(pActInfo);
		if (!pEmitter)
			return;

		const bool valuePort = IsPortActive(pActInfo, int(EInputs::Value));
		const bool triggerPort = IsPortActive(pActInfo, int(EInputs::Activate));
		if (!valuePort && !triggerPort)
			return;

		const string inputPropertyName = GetPortString(pActInfo, int(EInputs::Name));
		IParticleAttributes& attributes = pEmitter->GetAttributes();
		auto attributeId = attributes.FindAttributeIdByName(inputPropertyName.c_str());
		if (attributeId == -1)
		{
			GameWarning(
				"[flow] CFlowNode_ParticleAttributeSet: Cannot resolve attribute '%s' in entity '%s'",
				inputPropertyName.c_str(), pActInfo->pEntity->GetName());
			ActivateOutput(pActInfo, 0, 0);
			return;
		}

		switch (attributes.GetAttributeType(attributeId))
		{
		case IParticleAttributes::ET_Integer:
			attributes.SetAsInteger(attributeId, GetPortInt(pActInfo, int(int(EInputs::Value))));
			break;
		case IParticleAttributes::ET_Float:
			attributes.SetAsFloat(attributeId, GetPortFloat(pActInfo, int(int(EInputs::Value))));
			break;
		case IParticleAttributes::ET_Boolean:
			attributes.SetAsBoolean(attributeId, GetPortBool(pActInfo, int(int(EInputs::Value))));
			break;
		case IParticleAttributes::ET_Color:
			attributes.SetAsColor(attributeId, ColorF(GetPortVec3(pActInfo, int(int(EInputs::Value)))));
			break;
		}
	}
};
//-------------------------------------------------------------------------------------------------------------------------
// HPi begin
class CFlowNode_SpawnParticleEffect : public CFlowParticleClass
{
public:
	enum class EInputs
	{
		Spawn,
		DeSpawn,
		Enabled,
		Name,
		Pos,
		Dir,
		Scale,
		Loop,
		Duration,
		Prime,
	};

public:
	CFlowNode_SpawnParticleEffect(SActivationInfo* pActInfo):m_actInfo(*pActInfo)
	{
		m_bEmiterAlive = false;
	}
	~CFlowNode_SpawnParticleEffect()
	{
		if (m_bEmiterAlive && m_pEmitter)
		{
			m_bEmiterAlive = false;
			m_pEmitter->Kill();
		}
		RemovePendingTimers();
	}

	//-----------------------------------------------------------------------------
	//Timer
	//============================================================================
	void
		RemovePendingTimers()
	{
		// remove all old timers we have
		if (!gEnv->IsEditor() && !gEnv->pSystem->IsQuitting())
		{
			Activations::iterator iter;
			if (!m_activations.empty())
			{
				for (iter = m_activations.begin(); iter != m_activations.end(); ++iter)
				{
					gEnv->pGameFramework->RemoveTimer((*iter).first);
				}
				// clear in any case (Editor and Game mode)
				m_activations.clear();
			}
		}
		
		
	}
	//============================================================================
	void Serialize(SActivationInfo* pActInfo, TSerialize ser)
	{
		CTimeValue curTime = gEnv->pTimer->GetFrameStartTime();

		ser.BeginGroup("Local");
		// Editor mode: map with
		// key:  abs time in ms
		// data: SDelayData
		//
		// Game mode: map with
		// key:  timer id (we don't care about it)
		// data: SDelayData
		if (ser.IsWriting())
		{
			// when writing, currently relative values are stored!
			ser.Value("m_activations", m_activations);
#if 0
			CryLogAlways("CDelayNode write: current time(ms): %f", curTime.GetMilliSeconds());
			Activations::iterator iter = m_activations.begin();
			while (iter != m_activations.end())
			{
				CryLogAlways("CDelayNode write: ms=%d  timevalue(ms): %f", (*iter).first, (*iter).second.m_timeout.GetMilliSeconds());
				++iter;
			}
#endif
		} else
		{
			// FIXME: should we read the curTime from the file
			//        or is the FrameStartTime already set to the serialized value?
			// ser.Value("curTime", curTime);

			// when reading we have to differentiate between Editor and Game Mode
			if (gEnv->IsEditor())
			{
				// we can directly read into the m_activations array
				// regular update is handled by CFlowGraph
				ser.Value("m_activations", m_activations);
#if 0
				CryLogAlways("CDelayNode read: current time(ms): %f", curTime.GetMilliSeconds());
				Activations::iterator iter = m_activations.begin();
				while (iter != m_activations.end())
				{
					CryLogAlways("CDelayNode read: ms=%d  timevalue(ms): %f", (*iter).first, (*iter).second.m_timeout.GetMilliSeconds());
					++iter;
				}
#endif
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, !m_activations.empty());
			} else
			{
				RemovePendingTimers();
				// read serialized activations and re-register at timer
				Activations::iterator iter;
				Activations activations;
				ser.Value("m_activations", activations);
				for (iter = activations.begin(); iter != activations.end(); ++iter)
				{
					CTimeValue relTime = (*iter).second.m_timeout - curTime;
					IGameFramework::TimerID timerId = gEnv->pGameFramework->AddTimer(relTime, false, functor(CFlowNode_SpawnParticleEffect::OnTimer),
						this);
					m_activations[timerId] = (*iter).second;
				}
			}
		}
		ser.EndGroup();
	}
	//-----------------------------------------------------------------------------

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_SpawnParticleEffect(pActInfo);
	}

	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}

	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig inConfig[] =
		{
			InputPortConfig<SFlowSystemVoid>("Spawn",_HELP("Spawns Particle Effect")),
			InputPortConfig<SFlowSystemVoid>("DeSpawn",_HELP("De-Spawns Spawned Particle Effect")),
			InputPortConfig<bool>("Enabled",_HELP("Enable or Disable")),
			InputPortConfig<string>("Name", _HELP("Name Of Particle, Include Its Path")),
			InputPortConfig<Vec3>("Pos",_HELP("Position in World")),
			InputPortConfig<Vec3>("Dir",_HELP("Direction Of Effect")),
			InputPortConfig<float>("Scale",_HELP("Scale")),
			InputPortConfig<bool>("Loop",_HELP("Loop")),
			InputPortConfig<float>("Duration",_HELP("duration of spawn")),
			InputPortConfig<bool>("Prime",_HELP("Advance emitter age to its equilibrium state.")),
			{ 0 }
		};

		static const SOutputPortConfig outConfig[] =
		{
			OutputPortConfig_AnyType("Spawned", _HELP("Actives When Particle Successfully Spawned.")),
			OutputPortConfig_AnyType("DeSpawned", _HELP("Actives When Particle Successfully De-Spawned.")),
			OutputPortConfig_AnyType("Error", _HELP("Actives When Any Error Occurred.")),
			{ 0 }
		};

		config.sDescription = _HELP("Spawns and deSpawns Particle Effect In Game. an empty Entity can be targeted to this node to use its position and orientation");
		config.nFlags |= EFLN_TARGET_ENTITY;
		config.pInputPorts = inConfig;
		config.pOutputPorts = outConfig;
		config.SetCategory(EFLN_APPROVED);
	}

	virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
	{
		


		switch (event)
		{
		case eFE_Initialize:
			RemovePendingTimers();
			pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			break;
		case eFE_Activate:
	// we request a final activation if the input value changed
	// thus in case we get several inputs in ONE frame, we always only use the LAST one!
			if (IsPortActive(pActInfo, int(EInputs::Spawn)))
			{
				pActInfo->pGraph->RequestFinalActivation(pActInfo->myID);


				const bool spawnPort = IsPortActive(pActInfo, int(EInputs::Spawn));
				const bool enabledPort = IsPortActive(pActInfo, int(EInputs::Enabled));
				const bool namePort = IsPortActive(pActInfo, int(EInputs::Name));
				const bool posPort = IsPortActive(pActInfo, int(EInputs::Pos));
				const bool dirPort = IsPortActive(pActInfo, int(EInputs::Dir));
				const bool loopPort = IsPortActive(pActInfo, int(EInputs::Loop));
				const bool durationPort = IsPortActive(pActInfo, int(EInputs::Duration));
				const bool primePort = IsPortActive(pActInfo, int(EInputs::Prime));
				const bool scalePort = IsPortActive(pActInfo, int(EInputs::Scale));

				IEntity* pEntity = pActInfo->pEntity;
				if (pEntity)
				{
					if (!(namePort || posPort || spawnPort || enabledPort))
					{
						GameWarning(
							"[flow] CFlowNode_SpawnParticleEffect: At least one of the following ports must be active: Name, Pos, Spawn ,Enabled");
						ActivateOutput(pActInfo, 2, 1);
						return;
					}
				}



				const bool inputEnabled = GetPortBool(pActInfo, int(EInputs::Enabled));
				const string inputParticleName = GetPortString(pActInfo, int(EInputs::Name));
				Vec3 inputPos = pEntity ? pEntity->GetWorldPos() : GetPortVec3(pActInfo, int(EInputs::Pos));
				Vec3 inputDir = pEntity ? pEntity->GetWorldRotation().GetColumn1() : GetPortVec3(pActInfo, int(EInputs::Dir));//Up Direction
				const float inputScale = scalePort ? GetPortFloat(pActInfo, int(EInputs::Scale)) : 1.0f;
				const bool inputLoop = GetPortBool(pActInfo, int(EInputs::Loop));
				//const float inputDuration = durationPort ? GetPortFloat(pActInfo, int(EInputs::Duration)) : 1.0f;// 2 Seconds etc.
				const bool inputPrime = GetPortBool(pActInfo, int(EInputs::Prime));

				m_pParticleEffect = gEnv->pParticleManager->FindEffect(inputParticleName, "");
				if (!m_pParticleEffect)
				{
					GameWarning(
						"[flow] CFlowNode_SpawnParticleEffect: Cannot resolve particle effect '%s'",
						inputParticleName.c_str());
					ActivateOutput(pActInfo, 2, 1);
					return;
				}

				m_particleParams = m_pParticleEffect->GetParticleParams();
				//m_particleParams.fEmitterLifeTime = inputDuration;
				m_particleParams.bContinuous = inputLoop;
				m_particleParams.fSize = inputScale;
				m_pParticleEffect->SetParticleParams(m_particleParams);

				m_pParticleEffect->SetEnabled(inputEnabled);


				m_sp.bPrime = inputPrime;
				m_pEmitter = m_pParticleEffect->Spawn(ParticleLoc(inputPos, inputDir, inputScale), &m_sp);
				if (!m_pEmitter)
				{
					GameWarning(
						"[flow] CFlowNode_SpawnParticleEffect: Cannot spawn particle effect '%s'",
						inputParticleName.c_str());
					ActivateOutput(pActInfo, 2, 1);
					return;
				} else
				{
					if (pEntity)
					{
						m_pEmitter->SetEntity(pEntity, 0);
					}
					m_bEmiterAlive = true;
					ActivateOutput(pActInfo, 0, 1);
				}
			}

			if (IsPortActive(pActInfo, int(EInputs::DeSpawn)) && !(m_activations.empty()))
			{
				if (m_pEmitter && m_bEmiterAlive)
				{
					m_pEmitter->Kill();
					m_bEmiterAlive = false;
				}
				RemovePendingTimers();
				ActivateOutput(pActInfo, 1, 1);
				
				break;
			}
			break;
		case eFE_FinalActivate:
			{
				if (gEnv->IsEditor())
				{

					const float delay = GetPortFloat(pActInfo, int(EInputs::Duration));
					CTimeValue finishTime = gEnv->pTimer->GetFrameStartTime() + delay;
					m_activations[(int)finishTime.GetMilliSeconds()] = SDelayData(finishTime, GetPortAny(pActInfo, 0));
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				} else
				{

					const float delay = GetPortFloat(pActInfo, int(EInputs::Duration));
					CTimeValue finishTime = gEnv->pTimer->GetFrameStartTime() + delay;
					IGameFramework::TimerID timerId = gEnv->pGameFramework->AddTimer(delay, false, functor(CFlowNode_SpawnParticleEffect::OnTimer),
						this);
					m_activations[timerId] = SDelayData(finishTime, GetPortAny(pActInfo, 0));
				}
				break;
			}
		case eFE_Update:
			CRY_ASSERT(gEnv->IsEditor());
			CRY_ASSERT(!m_activations.empty());
			CTimeValue curTime = gEnv->pTimer->GetFrameStartTime();

			while (!m_activations.empty() && m_activations.begin()->second.m_timeout < curTime)
			{
				if (m_pEmitter && m_bEmiterAlive)
				{
					m_pEmitter->Kill();
					m_bEmiterAlive = false;
				}
				ActivateOutput(pActInfo, 1, 1);

				RemovePendingTimers();
				m_activations.erase(m_activations.begin());
			}
			if (m_activations.empty())
				pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, false);
			break;
		}
	}
//======================================================================================================
static void OnTimer(void* pUserData, IGameFramework::TimerID ref)
	{
		CRY_ASSERT(gEnv->IsEditor() == false);

		CFlowNode_SpawnParticleEffect* pThis = static_cast<CFlowNode_SpawnParticleEffect*>(pUserData);
		Activations::iterator iter = pThis->m_activations.find(ref);
		if (iter == pThis->m_activations.end())
		{
			GameWarning("CFlowDelayNode::OnTimer: Stale reference %d", ref);
			return;
		}

		if (pThis->m_pEmitter)
		{
			//pThis->m_pParticleEffect->Release();
			pThis->m_pEmitter->Kill();
			pThis->m_bEmiterAlive = false;
		}
		pThis->RemovePendingTimers();
		pThis->m_activations.erase(iter);
		ActivateOutput(&pThis->m_actInfo, 1, iter->second.m_data);

		
	}

private:
	struct SDelayData
	{
		SDelayData()
		{
		}
		SDelayData(const CTimeValue& timeout, const TFlowInputData& data)
			: m_timeout(timeout), m_data(data)
		{
		}

		CTimeValue     m_timeout;
		TFlowInputData m_data;
		bool operator<(const SDelayData& rhs) const
		{
			return m_timeout < rhs.m_timeout;
		}

		void Serialize(TSerialize ser)
		{
			ser.Value("m_timeout", m_timeout);
			ser.Value("m_data", m_data);
		}

		void GetMemoryUsage(ICrySizer* pSizer) const
		{ /*nothing*/
		}
	};
	IParticleEffect* m_pParticleEffect;
	IParticleEmitter* m_pEmitter;
	ParticleParams m_particleParams;
	SpawnParams m_sp;

	SActivationInfo m_actInfo;
	typedef std::map<IGameFramework::TimerID, SDelayData> Activations;
	Activations m_activations;
//	static void OnTimer(void* pUserData, IGameFramework::TimerID ref);
	bool m_bEmiterAlive;

};
REGISTER_FLOW_NODE("Particle:SpawnParticleEffect", CFlowNode_SpawnParticleEffect)
REGISTER_FLOW_NODE("Particle:AttributeGet", CFlowNode_ParticleAttributeGet)
REGISTER_FLOW_NODE("Particle:AttributeSet", CFlowNode_ParticleAttributeSet)
