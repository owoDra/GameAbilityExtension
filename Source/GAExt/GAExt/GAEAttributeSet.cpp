// Copyright (C) 2023 owoDra

#include "GAEAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAEAttributeSet)


UGAEAttributeSet::UGAEAttributeSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UWorld* UGAEAttributeSet::GetWorld() const
{
	const auto* Outer{ GetOuter() };
	check(Outer);

	return Outer->GetWorld();
}
