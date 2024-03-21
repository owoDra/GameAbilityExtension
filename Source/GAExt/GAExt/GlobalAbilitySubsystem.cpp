// Copyright (C) 2024 owoDra

#include "GlobalAbilitySubsystem.h"

#include "GAExtLogs.h"

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

	UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("|| + ASC: %s, Owner: %s, Ability: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()), *GetNameSafe(Ability));

	Handles.Add(ASC, AbilitySpecHandle);
}

void FGlobalAppliedAbilityList::RemoveFromASC(UAbilitySystemComponent* ASC)
{
	if (auto* SpecHandle{ Handles.Find(ASC) })
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("|| - ASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

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
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("|| - ASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

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

	const auto* GameplayEffectCDO{ Effect.GetDefaultObject() };
	const auto GameplayEffectHandle{ ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, /*Level=*/ 1, ASC->MakeEffectContext(), ASC->GetPredictionKeyForNewAction()) };
	
	UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("|| + ASC: %s, Owner: %s, Effect: %s, Handle: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()), *GetNameSafe(Effect), *GameplayEffectHandle.ToString());

	Handles.Add(ASC, GameplayEffectHandle);
}

void FGlobalAppliedEffectList::RemoveFromASC(UAbilitySystemComponent* ASC)
{
	if (auto* EffectHandle{ Handles.Find(ASC) })
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("|| - ASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

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
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("|| - ASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

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
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("ApplyAbilityToAll: %s"), *GetNameSafe(Ability));
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [AppliedAbilities][%d]"), AppliedAbilities.Num());

		auto& Entry{ AppliedAbilities.Add(Ability) };

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| -> [AppliedAbilities][%d]"), AppliedAbilities.Num());

		for (const auto& ASC : RegisteredASCs)
		{
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| -> ASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

			Entry.AddToASC(Ability, ASC);
		}
	}
}

void UGlobalAbilitySubsystem::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if (Effect && (!AppliedEffects.Contains(Effect)))
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("ApplyEffectToAll: %s"), *GetNameSafe(Effect));
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [AppliedEffects][%d]"), AppliedEffects.Num());

		auto& Entry{ AppliedEffects.Add(Effect) };

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| -> [AppliedEffects][%d]"), AppliedEffects.Num());

		for (const auto& ASC : RegisteredASCs)
		{
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| -> ASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

			Entry.AddToASC(Effect, ASC);
		}
	}
}

void UGlobalAbilitySubsystem::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if (Ability && AppliedAbilities.Contains(Ability))
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("RemoveAbilityFromAll: %s"), *GetNameSafe(Ability));
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [AppliedAbilities][%d]"), AppliedAbilities.Num());

		auto& Entry{ AppliedAbilities[Ability] };
		Entry.RemoveFromAll();
		AppliedAbilities.Remove(Ability);

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| -> [AppliedAbilities][%d]"), AppliedAbilities.Num());
	}
}

void UGlobalAbilitySubsystem::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if (Effect && AppliedEffects.Contains(Effect))
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("RemoveEffectFromAll: %s"), *GetNameSafe(Effect));
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [AppliedEffects][%d]"), AppliedEffects.Num());

		auto& Entry{ AppliedEffects[Effect] };
		Entry.RemoveFromAll();
		AppliedEffects.Remove(Effect);

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| -> [AppliedEffects][%d]"), AppliedEffects.Num());
	}
}

void UGlobalAbilitySubsystem::RegisterASC(UAbilitySystemComponent* ASC)
{
	if (ensure(ASC) && !RegisteredASCs.Contains(ASC))
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("RegisterASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [Ability][%d]"), AppliedAbilities.Num());
		for (auto& Entry : AppliedAbilities)
		{
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| + %s"), *GetNameSafe(Entry.Key));

			Entry.Value.AddToASC(Entry.Key, ASC);
		}

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [Effect][%d]"), AppliedEffects.Num());
		for (auto& Entry : AppliedEffects)
		{
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| + %s"), *GetNameSafe(Entry.Key));

			Entry.Value.AddToASC(Entry.Key, ASC);
		}

		RegisteredASCs.AddUnique(ASC);
	}
}

void UGlobalAbilitySubsystem::UnregisterASC(UAbilitySystemComponent* ASC)
{
	if (ensure(ASC) && RegisteredASCs.Contains(ASC))
	{
		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("UnregisterASC: %s, Owner: %s"), *GetNameSafe(ASC), *GetNameSafe(ASC->GetOwner()));

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [Ability][%d]"), AppliedAbilities.Num());
		for (auto& Entry : AppliedAbilities)
		{
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| - %s"), *GetNameSafe(Entry.Key));

			Entry.Value.RemoveFromASC(ASC);
		}

		UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| [Effect][%d]"), AppliedEffects.Num());
		for (auto& Entry : AppliedEffects)
		{
			UE_LOG(LogGameExt_GlobalAbility, Log, TEXT("| - %s"), *GetNameSafe(Entry.Key));

			Entry.Value.RemoveFromASC(ASC);
		}

		RegisteredASCs.Remove(ASC);
	}
}

#pragma endregion
