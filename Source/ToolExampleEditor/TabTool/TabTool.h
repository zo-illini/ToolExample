#pragma once

#include "ToolExampleEditor/ExampleTabToolBase.h"
#include "Animation/AnimSequence.h"
#include "AssetThumbnail.h"
#include "AssetRegistryModule.h"
#include "PropertyEditing.h"

class TabTool : public FExampleTabToolBase
{
public:
	virtual ~TabTool() {}
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;
	virtual void Initialize() override;
	virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) override;

protected:
	UAnimSequence * AnimSequence;

	
	TSharedPtr<SWidget> SelectAnimSequenceWidgetPtr;
	TSharedPtr<SComboButton> SelectAnimSequenceButtonPtr;
	TSharedPtr<FAssetThumbnail> AnimThumbnailPtr;
	TSharedPtr<FAssetThumbnailPool> AnimThumbnailPoolPtr;
	TSharedPtr<SWidget> AnimSequencePicker;

	FReply OnAnimButtonClicked();
	TSharedRef<SWidget> OnGetAnimMenu();
	void OnAnimAssetSelected(const FAssetData& AssetData);

	FName BoneName;

	TArray<FTransform> BoneTransform;

	FReply ExtractBoneCurve();
	FText GetBoneName() const;
	void OnBoneNameCommitted(const FText& InText, ETextCommit::Type CommitInfo);

	bool FillBoneTransform();
	void FillCurveVector();
};