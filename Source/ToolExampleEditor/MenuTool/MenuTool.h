#pragma once

#include "ToolExampleEditor/IExampleModuleInterface.h"
#include "Animation/AnimSequence.h"
#include "AssetThumbnail.h"
#include "AssetRegistryModule.h"
#include "PropertyEditing.h"

class MenuTool : public IExampleModuleListenerInterface, public TSharedFromThis<MenuTool>
{
public:
	virtual ~MenuTool() {}

	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

	void MakeMenuEntry(FMenuBuilder &menuBuilder);
	void MakeSubMenu(FMenuBuilder &menuBuilder);

	UAnimSequence * AnimSequence;


protected:
	TSharedPtr<FUICommandList> CommandList;
	TSharedPtr<SWidget> SelectAnimSequenceWidgetPtr;
	TSharedPtr<SComboButton> SelectAnimSequenceButtonPtr;
	TSharedPtr<FAssetThumbnail> AnimThumbnailPtr;
	TSharedPtr<FAssetThumbnailPool> AnimThumbnailPoolPtr;
	TSharedPtr<SWidget> AnimSequencePicker;

	
	void MapCommands();

	//************************
	// UI Command functions
	void MenuCommand1();
	void MenuCommand2();
	void MenuCommand3();

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