#pragma once

#include "NiagaraCommon.h"
#include "NiagaraCollision.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraShared.h"
#include "VectorVM.h"
#include "NiagaraDataInterface.h"
#include "HAL/PlatformAtomics.h"
#include "NiagaraDataInterfacePostProcess.generated.h"

class INiagaraCompiler;
class FNiagaraSystemInstance;




/** Data Interface allowing a counter that starts at zero when created and increases every time it is sampled. Note that for now this is a signed integer, meaning that it can go negative when it loops past INT_MAX. */
UCLASS(EditInlineNew, Category = "PostProcess", meta = (DisplayName = "Post Process Control"))
class UNiagaraDataInterfacePostProcess : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Setting")
	float	Radius;

	UPROPERTY(EditAnywhere, Category = "Setting")
	FPostProcessSettings	Settings;

	UPROPERTY(EditAnywhere, Category = "Setting")
	FString VariableName;

	UPROPERTY(EditAnywhere, Category = "Setting")
	FNiagaraVariable Variable;


	struct FPerInstanceData
	{
		float Amount;		//Amount on runtime
		float ParameterB;	//Unused
		float ParameterC;	//Unused
		float ParameterD;	//Unused
	};
	//UObject Interface
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//UObject Interface End

	/** Initializes the per instance data for this interface. Returns false if there was some error and the simulation should be disabled. */
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance) override;
	/** Destroys the per instence data for this interface. */
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance) {}

	/** Ticks the per instance data for this interface, if it has any. */
	virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds);
	virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds);

	virtual int32 PerInstanceDataSize()const { return sizeof(FPerInstanceData); }	//if Instance size == 0, PerInstanceTick is never called.


	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)override;
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc) override;

	// VM functions
	void SetPostProcessParameters(FVectorVMContext& Context);

	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return true; }

	virtual bool GetFunctionHLSL(const FName&  DefinitionFunctionName, FString InstanceFunctionName, FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual void GetParameterDefinitionHLSL(FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;

	virtual void ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance) override
	{
		check(false);
	}

protected:
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

private:
};
