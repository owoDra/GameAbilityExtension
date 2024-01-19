// Copyright (C) 2024 owoDra

#include "AbilitySet.h"

#include "GAExtLogs.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayAbilitySpec.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilitySet)

///////////////////////////////////////////////////////////////
// FAbilitySet_GameplayAbility

bool FAbilitySet_GameplayAbility::IsValid() const
{
	return ::IsValid(Ability);
}


///////////////////////////////////////////////////////////////
// FAbilitySet_GameplayEffect

bool FAbilitySet_GameplayEffect::IsValid() const
{
	return ::IsValid(GameplayEffect);
}


///////////////////////////////////////////////////////////////
// FAbilitySet_AttributeSet

bool FAbilitySet_AttributeSet::IsValid() const
{
	return ::IsValid(AttributeSet);
}


///////////////////////////////////////////////////////////////
// FAbilitySet_GrantedHandles

#pragma region FAbilitySet_GrantedHandles

void FAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}


void FAbilitySet_GrantedHandles::AddAbilities(UAbilitySystemComponent* ASC, const TArray<FAbilitySet_GameplayAbility>& Abilities, UObject* SourceObject)
{
	if (ensure(ASC))
	{
		// Must be authoritative to give or take ability sets.

		if (!ASC->IsOwnerActorAuthoritative())
		{
			return;
		}

		// Grant the gameplay abilities.

		for (int32 AbilityIndex{ 0 }; AbilityIndex < Abilities.Num(); ++AbilityIndex)
		{
			const auto& AbilityToGrant{ Abilities[AbilityIndex] };

			if (!AbilityToGrant.Ability)
			{
				UE_LOG(LogGAE, Error, TEXT("Direct adding GrantedGameplayAbilities[%d] is not valid."), AbilityIndex);
				continue;
			}

			auto* AbilityCDO{ AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>() };

			auto AbilitySpec{ FGameplayAbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel) };
			AbilitySpec.SourceObject = SourceObject;
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

			const auto AbilitySpecHandle{ ASC->GiveAbility(AbilitySpec) };

			AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}

void FAbilitySet_GrantedHandles::AddGameplayEffects(UAbilitySystemComponent* ASC, const TArray<FAbilitySet_GameplayEffect>& Effects, UObject* SourceObject)
{
	if (ensure(ASC))
	{
		// Must be authoritative to give or take ability sets.

		if (!ASC->IsOwnerActorAuthoritative())
		{
			return;
		}

		// Grant the gameplay effects.

		for (int32 EffectIndex{ 0 }; EffectIndex < Effects.Num(); ++EffectIndex)
		{
			const auto& EffectToGrant{ Effects[EffectIndex] };

			if (!EffectToGrant.GameplayEffect)
			{
				UE_LOG(LogGAE, Error, TEXT("Direct Adding GrantedGameplayEffects[%d] is not valid"), EffectIndex);
				continue;
			}

			const auto* GameplayEffect{ EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>() };
			const auto GameplayEffectHandle{ ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext()) };

			AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}

void FAbilitySet_GrantedHandles::AddAttributeSets(UAbilitySystemComponent* ASC, const TArray<FAbilitySet_AttributeSet>& Sets, UObject* SourceObject)
{
	if (ensure(ASC))
	{
		// Must be authoritative to give or take ability sets.

		if (!ASC->IsOwnerActorAuthoritative())
		{
			return;
		}

		for (int32 SetIndex = 0; SetIndex < Sets.Num(); ++SetIndex)
		{
			const auto& SetToGrant{ Sets[SetIndex] };

			if (!IsValid(SetToGrant.AttributeSet))
			{
				UE_LOG(LogGAE, Error, TEXT("Direct adding GrantedAttributes[%d] is not valid"), SetIndex);
				continue;
			}

			auto* NewSet{ NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet) };
			ASC->AddAttributeSetSubobject(NewSet);

			AddAttributeSet(NewSet);
		}
	}
}


void FAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (ensure(ASC))
	{
		// Must be authoritative to give or take ability sets.

		if (!ASC->IsOwnerActorAuthoritative())
		{
			return;
		}

		for (const auto& Handle : AbilitySpecHandles)
		{
			if (Handle.IsValid())
			{
				ASC->ClearAbility(Handle);
			}
		}

		for (const auto& Handle : GameplayEffectHandles)
		{
			if (Handle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(Handle);
			}
		}

		for (const auto& Set : GrantedAttributeSets)
		{
			ASC->RemoveSpawnedAttribute(Set);
		}

		AbilitySpecHandles.Reset();
		GameplayEffectHandles.Reset();
		GrantedAttributeSets.Reset();
	}
}

#pragma endregion


///////////////////////////////////////////////////////////////
// UAbilitySet

#pragma region UAbilitySet

UAbilitySet::UAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR 
EDataValidationResult UAbilitySet::IsDataValid(FDataValidationContext& Context) const
{
	auto Result{ CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid) };

	int32 Index{ 0 };
	for (const auto& Ability : GrantedGameplayAbilities)
	{
		if (!Ability.IsValid())
		{
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);

			Context.AddError(FText::FromString(FString::Printf(TEXT("Invalid Ability set in GrantedGameplayAbilities[%d] in %s"), Index, *GetNameSafe(this))));
		}
	}

	Index = 0;
	for (const auto& Effect : GrantedGameplayEffects)
	{
		if (!Effect.IsValid())
		{
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);

			Context.AddError(FText::FromString(FString::Printf(TEXT("Invalid Effect set in GrantedGameplayEffects[%d] in %s"), Index, *GetNameSafe(this))));
		}
	}

	Index = 0;
	for (const auto& Attribute : GrantedAttributes)
	{
		if (!Attribute.IsValid())
		{
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);

			Context.AddError(FText::FromString(FString::Printf(TEXT("Invalid Attribute set in GrantedAttributes[%d] in %s"), Index, *GetNameSafe(this))));
		}
	}

	return Result;
}
#endif


void UAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, FAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	if (ensure(ASC))
	{
		// Must be authoritative to give or take ability sets.

		if (!ASC->IsOwnerActorAuthoritative())
		{
			return;
		}

		// Grant the gameplay abilities.

		for (int32 AbilityIndex{ 0 }; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
		{
			const auto& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

			if (!AbilityToGrant.Ability)
			{
				UE_LOG(LogGAE, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
				continue;
			}

			auto* AbilityCDO{ AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>() };

			auto AbilitySpec{ FGameplayAbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel) };
			AbilitySpec.SourceObject = SourceObject;
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

			const auto AbilitySpecHandle{ ASC->GiveAbility(AbilitySpec) };

			if (OutGrantedHandles)
			{
				OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
			}
		}

		// Grant the gameplay effects.

		for (int32 EffectIndex{ 0 }; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
		{
			const auto& EffectToGrant{ GrantedGameplayEffects[EffectIndex] };

			if (!EffectToGrant.GameplayEffect)
			{
				UE_LOG(LogGAE, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
				continue;
			}

			const auto* GameplayEffect{ EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>() };
			const auto GameplayEffectHandle{ ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext()) };

			if (OutGrantedHandles)
			{
				OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
			}
		}

		// Grant the attribute sets.

		for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
		{
			const auto& SetToGrant{ GrantedAttributes[SetIndex] };

			if (!IsValid(SetToGrant.AttributeSet))
			{
				UE_LOG(LogGAE, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
				continue;
			}

			auto* NewSet{ NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet) };
			ASC->AddAttributeSetSubobject(NewSet);

			if (OutGrantedHandles)
			{
				OutGrantedHandles->AddAttributeSet(NewSet);
			}
		}
	}
}

void UAbilitySet::BP_GiveToAbilitySystem(UAbilitySystemComponent* ASC, FAbilitySet_GrantedHandles& OutGrantedHandles, UObject* SourceObject) const
{
	GiveToAbilitySystem(ASC, &OutGrantedHandles, SourceObject);
}

#pragma endregion
