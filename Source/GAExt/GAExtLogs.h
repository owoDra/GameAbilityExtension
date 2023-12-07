// Copyright (C) 2023 owoDra

#pragma once

#include "Logging/LogMacros.h"

GAEXT_API DECLARE_LOG_CATEGORY_EXTERN(LogGAE, Log, All);

#if !UE_BUILD_SHIPPING

#define GAELOG(FormattedText, ...) UE_LOG(LogGAE, Log, FormattedText, __VA_ARGS__)

#define GAEENSURE(InExpression) ensure(InExpression)
#define GAEENSURE_MSG(InExpression, InFormat, ...) ensureMsgf(InExpression, InFormat, __VA_ARGS__)
#define GAEENSURE_ALWAYS_MSG(InExpression, InFormat, ...) ensureAlwaysMsgf(InExpression, InFormat, __VA_ARGS__)

#define GAECHECK(InExpression) check(InExpression)
#define GAECHECK_MSG(InExpression, InFormat, ...) checkf(InExpression, InFormat, __VA_ARGS__)

#else

#define GAELOG(FormattedText, ...)

#define GAEENSURE(InExpression) InExpression
#define GAEENSURE_MSG(InExpression, InFormat, ...) InExpression
#define GAEENSURE_ALWAYS_MSG(InExpression, InFormat, ...) InExpression

#define GAECHECK(InExpression) InExpression
#define GAECHECK_MSG(InExpression, InFormat, ...) InExpression

#endif