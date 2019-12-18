// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "INiagaraPostProcess.h"


class FNiagaraPostProcess : public INiagaraPostProcess
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FNiagaraPostProcess, NiagaraPostProcess )



void FNiagaraPostProcess::StartupModule()
{
}


void FNiagaraPostProcess::ShutdownModule()
{
}



