// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NiagaraDataInterfacePostProcess.h"
#include "NiagaraTypes.h"
#include "NiagaraCustomVersion.h"
#include "Internationalization/Internationalization.h"
#include "NiagaraSystemSimulation.h"
#include "Components/PostProcessComponent.h"
#include "Components/SphereComponent.h"

#define LOCTEXT_NAMESPACE "NiagaraDataInterfaceMyNiagaraData"
DEFINE_LOG_CATEGORY_STATIC(LogNiagaraPostProcess, Log, All);


//////////////////////////////////////////////////////////////////////////
//Color Curve


UNiagaraDataInterfacePostProcess::UNiagaraDataInterfacePostProcess(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Radius = 500.0f;

}

bool UNiagaraDataInterfacePostProcess::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance)
{
	FPerInstanceData *PIData = new (PerInstanceData) FPerInstanceData;
	PIData->Amount = 0.0f;
	PIData->ParameterB = 0.0f;
	PIData->ParameterC = 0.0f;
	PIData->ParameterD = 0.0f;
	return true;
}

void UNiagaraDataInterfacePostProcess::PostInitProperties()
{
	Super::PostInitProperties();

	//Can we register data interfaces as regular types and fold them into the FNiagaraVariable framework for UI and function calls etc?
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), true, false, false);
	}
}

void UNiagaraDataInterfacePostProcess::PostLoad()
{
	Super::PostLoad();

	const int32 NiagaraVer = GetLinkerCustomVersion(FNiagaraCustomVersion::GUID);
}

#if WITH_EDITOR

void UNiagaraDataInterfacePostProcess::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif


bool UNiagaraDataInterfacePostProcess::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}
	UNiagaraDataInterfacePostProcess* OtherTyped = CastChecked<UNiagaraDataInterfacePostProcess>(Destination);


	OtherTyped->Settings = Settings;
	OtherTyped->Radius = Radius;
	return true;
}

static bool operator==(const FPostProcessSettings& lhs, const FPostProcessSettings& rhs)
{
	if( lhs.bOverride_ColorContrast != rhs.bOverride_ColorContrast )return false;
	if( lhs.ColorContrast != rhs.ColorContrast )return false;
	return true;
}

bool UNiagaraDataInterfacePostProcess::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UNiagaraDataInterfacePostProcess* OtherTyped = CastChecked<const UNiagaraDataInterfacePostProcess>(Other);

	return 
		OtherTyped->Settings == Settings &&
		OtherTyped->Radius == Radius &&
		true;
}


void UNiagaraDataInterfacePostProcess::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	FNiagaraFunctionSignature Sig3;
	Sig3.Name = TEXT("SetPostProcessParameters");
	Sig3.bMemberFunction = true;
	Sig3.bRequiresContext = false;
	Sig3.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("MyData")));
	Sig3.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Amount")));
	Sig3.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("B")));
	Sig3.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("C")));
	Sig3.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("D")));
	Sig3.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
	Sig3.SetDescription(LOCTEXT("Pass parameters to the post process component", "see NiagaraDataInterfacePostProcess"));
	OutFunctions.Add(Sig3);
}

bool UNiagaraDataInterfacePostProcess::GetFunctionHLSL(const FName& DefinitionFunctionName, FString InstanceFunctionName, FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	return false;
}

void UNiagaraDataInterfacePostProcess::GetParameterDefinitionHLSL(FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
}

void UNiagaraDataInterfacePostProcess::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc)
{
//	FMyNiagaraDataInstanceData *InstData = (FMyNiagaraDataInstanceData *)InstanceData;
	if (BindingInfo.Name == TEXT("SetPostProcessParameters"))
	{
		OutFunc = FVMExternalFunction::CreateUObject(this, &UNiagaraDataInterfacePostProcess::SetPostProcessParameters);
	}
	else
	{
		UE_LOG(LogNiagaraPostProcess, Error, TEXT("Could not find data interface external function. %s\n"),
			*BindingInfo.Name.ToString());
	}
}

void UNiagaraDataInterfacePostProcess::SetPostProcessParameters(FVectorVMContext& Context)
{
	VectorVM::FExternalFuncInputHandler<float> ParamA(Context);
	VectorVM::FExternalFuncInputHandler<float> ParamB(Context);
	VectorVM::FExternalFuncInputHandler<float> ParamC(Context);
	VectorVM::FExternalFuncInputHandler<float> ParamD(Context);

	VectorVM::FUserPtrHandler<FPerInstanceData> InstanceData(Context);

	VectorVM::FExternalFuncRegisterHandler<bool> OutValue(Context);

	for (int32 i = 0; i < Context.NumInstances; ++i)
	{
		InstanceData->Amount		= ParamA.GetAndAdvance();
		InstanceData->ParameterB	= ParamB.GetAndAdvance();
		InstanceData->ParameterC	= ParamC.GetAndAdvance();
		InstanceData->ParameterD	= ParamD.GetAndAdvance();
		*OutValue.GetDest()			= true;
		OutValue.Advance();
	}
}


bool UNiagaraDataInterfacePostProcess::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance, float DeltaSeconds)
{
#if 0
	UNiagaraComponent* Component = InSystemInstance->GetComponent();
	AActor* Actor = Component ? Component->GetOwner() : nullptr;
	float A = InSystemInstance->GetSystemSimulation()->GetUpdateExecutionContext().Parameters.GetParameterValue<float>(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("NE_Burst.AAA")));
	float B = InSystemInstance->GetSystemSimulation()->GetUpdateExecutionContext().Parameters.GetParameterValue<float>(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("NE_Burst.ABC")));
	float C = InSystemInstance->GetSystemSimulation()->GetUpdateExecutionContext().Parameters.GetParameterValue<float>(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Emitter.AAA")));
	float D = InSystemInstance->GetSystemSimulation()->GetUpdateExecutionContext().Parameters.GetParameterValue<float>(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Emitter.ABC")));
//	InSystemInstance->GetSystemSimulation()->GetUpdateExecutionContext().DataSetInfo.
	FNiagaraDataSetAccessor<float> ABCAccessor(InSystemInstance->GetSystemSimulation()->GetMainDataSet(), FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("NE_Burst.ABC")) );
	float E = ABCAccessor.GetSafe(0, 0.0f);

	UE_LOG(LogNiagaraPostProcess, Verbose, TEXT("Age %8.2f %20s %20s %6.2f %6.2f %6.2f %6.2f %6.2f"), InSystemInstance->GetAge(), *Component->GetName(), Actor ? *Actor->GetName() : TEXT("None"), A, B, C, D, E);
#endif
	return false;
}

bool UNiagaraDataInterfacePostProcess::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance, float DeltaSeconds)
{
	FPerInstanceData *PIData = static_cast<FPerInstanceData*>(PerInstanceData);
	UE_LOG(LogNiagaraPostProcess, Verbose, TEXT("A %8.2f B %8.2f C %8.2f D %8.2f"), PIData->Amount, PIData->ParameterB, PIData->ParameterC, PIData->ParameterD);

	UNiagaraComponent* Component = InSystemInstance->GetComponent();
	AActor* Actor = Component ? Component->GetOwner() : nullptr;
	if( Actor )
	{
		UPostProcessComponent* PostProcess = Actor->FindComponentByClass<UPostProcessComponent>();
		if( PostProcess == nullptr )
		{
			USphereComponent* Sphere = Cast<USphereComponent>( NewObject<USphereComponent>(Actor,FName("Sphere") ) );
			Sphere->SetSphereRadius( Radius );
			Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Sphere->RegisterComponent();
			Sphere->AttachToComponent(Actor->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);

			PostProcess = Cast<UPostProcessComponent>( NewObject<UPostProcessComponent>(Actor,FName("PostProcess") ) );
			PostProcess->bUnbound = false;
			PostProcess->RegisterComponent();
			PostProcess->AttachToComponent(Sphere, FAttachmentTransformRules::KeepRelativeTransform);
		}
		USphereComponent* Sphere = Cast<USphereComponent>( PostProcess->GetAttachParent() );
		if( Sphere )
		{
			Sphere->SetSphereRadius( Radius );
		}
		PostProcess->Settings = Settings;
		PostProcess->BlendWeight = FMath::Clamp(PIData->Amount,0.0f,1.0f);
	}

	return false;
}

#undef LOCTEXT_NAMESPACE