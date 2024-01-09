// Copyright (C) 2024 owoDra

#pragma once

#include "AttributeSet.h"

#include "GAEAttributeSet.generated.h"

struct FGameplayEffectSpec;


/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UBEHealthSet, Health)
 * 
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * Delegate used to broadcast attribute events.
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec& /*EffectSpec*/, float /*EffectMagnitude*/);


/**
 * AttributeSet with simple utilities added
 */
UCLASS()
class GAEXT_API UGAEAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UGAEAttributeSet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UWorld* GetWorld() const override;

	template<typename T>
	T* GetAbilitySystemComponent() const
	{
		return Cast<T>(GetOwningAbilitySystemComponent());
	}

};
