// Copyright (C) 2023 owoDra

#include "GlobalAbilitySubsystem.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayAbilitySpec.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalAbilitySubsystem)


#pragma region GlobalAppliedAbilityList

void FGlobalAppliedAbilityList::AddToASC(TSubclassOf<UGameplayAbility> Ability, UAbilitySystemComponent* ASC)
{
	// Removing global abilities that are being applied

	RemoveFromASC(ASC);

	// Add new global abilities

	auto* AbilityCDO{ Ability->GetDefaultObject<UGameplayAbility>() };
	auto AbilitySpec{ FGameplayAbilitySpec(AbilityCDO) };
	const auto AbilitySpecHandle{ ASC->GiveAbility(AbilitySpec) };

	Handles.Add(ASC, AbilitySpecHandle);
}

void FGlobalAppliedAbilityList::RemoveFromASC(UAbilitySystemComponent* ASC)
{
	if (auto* SpecHandle{ Handles.Find(ASC) })
	{
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}

void FGlobalAppliedAbilityList::RemoveFromAll()
{
	for (const auto& KVP : Handles)
	{
		const auto& ASC{ KVP.Key };
		const auto& Handle{ KVP.Value };

		if (ASC)
		{
			ASC->ClearAbility(Handle);
		}
	}

	Handles.Empty();
}

#pragma endregion


#pragma region GlobalAppliedEffectList

void FGlobalAppliedEffectList::AddToASC(TSubclassOf<UGameplayEffect> Effect, UAbilitySystemComponent* ASC)
{
	// Removing global effects that are being applied

	RemoveFromASC(ASC);

	// Add new global effects

	const auto* GameplayEffectCDO{ Effect->GetDefaultObject<UGameplayEffect>() };
	const auto GameplayEffectHandle{ ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, /*Level=*/ 1, ASC->MakeEffectContext()) };

	Handles.Add(ASC, GameplayEffectHandle);
}

void FGlobalAppliedEffectList::RemoveFromASC(UAbilitySystemComponent* ASC)
{
	if (auto* EffectHandle{ Handles.Find(ASC) })
	{
		ASC->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(ASC);
	}
}

void FGlobalAppliedEffectList::RemoveFromAll()
{
	for (const auto& KVP : Handles)
	{
		const auto& ASC{ KVP.Key };
		const auto& Handle{ KVP.Value };

		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	Handles.Empty();
}

#pragma endregion


#pragma region GlobalAbilitySubsystem

void UGlobalAbilitySubsystem::ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability)
{
	if (Ability && (!AppliedAbilities.Contains(Ability)))
	{
		auto& Entry{ AppliedAbilities.Add(Ability) };

		for (const auto& ASC : RegisteredASCs)
		{
			Entry.AddToASC(Ability, ASC);
		}
	}
}

void UGlobalAbilitySubsystem::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if (Effect && (!AppliedEffects.Contains(Effect)))
	{
		auto& Entry{ AppliedEffects.Add(Effect) };

		for (const auto& ASC : RegisteredASCs)
		{
			Entry.AddToASC(Effect, ASC);
		}
	}
}

void UGlobalAbilitySubsystem::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if (Ability && AppliedAbilities.Contains(Ability))
	{
		auto& Entry{ AppliedAbilities[Ability] };
		Entry.RemoveFromAll();
		AppliedAbilities.Remove(Ability);
	}
}

void UGlobalAbilitySubsystem::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if (Effect && AppliedEffects.Contains(Effect))
	{
		auto& Entry{ AppliedEffects[Effect] };
		Entry.RemoveFromAll();
		AppliedEffects.Remove(Effect);
	}
}

void UGlobalAbilitySubsystem::RegisterASC(UAbilitySystemComponent* ASC)
{
	if (ensure(ASC))
	{
		for (auto& Entry : AppliedAbilities)
		{
			Entry.Value.AddToASC(Entry.Key, ASC);
		}

		for (auto& Entry : AppliedEffects)
		{
			Entry.Value.AddToASC(Entry.Key, ASC);
		}

		RegisteredASCs.AddUnique(ASC);
	}
}

void UGlobalAbilitySubsystem::UnregisterASC(UAbilitySystemComponent* ASC)
{
	if (ensure(ASC))
	{
		for (auto& Entry : AppliedAbilities)
		{
			Entry.Value.RemoveFromASC(ASC);
		}

		for (auto& Entry : AppliedEffects)
		{
			Entry.Value.RemoveFromASC(ASC);
		}

		RegisteredASCs.Remove(ASC);
	}
}

#pragma endregion
