#include "ToolExampleEditor/ToolExampleEditor.h"
#include "MenuTool.h"


#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "MenuTool"

class MenuToolCommands : public TCommands<MenuToolCommands>
{
public:

	MenuToolCommands()
		: TCommands<MenuToolCommands>(
		TEXT("MenuTool"), // Context name for fast lookup
		FText::FromString("Example Menu tool"), // Context name for displaying
		NAME_None,	 // No parent context
		FEditorStyle::GetStyleSetName() // Icon Style Set
		)
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(MenuCommand1, "Menu Command 1", "Test Menu Command 1.", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(MenuCommand2, "Menu Command 2", "Test Menu Command 2.", EUserInterfaceActionType::Button, FInputGesture());
		UI_COMMAND(MenuCommand3, "Menu Command 3", "Test Menu Command 3.", EUserInterfaceActionType::Button, FInputGesture());

	}

public:
	TSharedPtr<FUICommandInfo> MenuCommand1;
	TSharedPtr<FUICommandInfo> MenuCommand2;
	TSharedPtr<FUICommandInfo> MenuCommand3;
};

void MenuTool::MapCommands()
{
	const auto& Commands = MenuToolCommands::Get();

	CommandList->MapAction(
		Commands.MenuCommand1,
		FExecuteAction::CreateSP(this, &MenuTool::MenuCommand1),
		FCanExecuteAction());

	CommandList->MapAction(
		Commands.MenuCommand2,
		FExecuteAction::CreateSP(this, &MenuTool::MenuCommand2),
		FCanExecuteAction());

	CommandList->MapAction(
		Commands.MenuCommand3,
		FExecuteAction::CreateSP(this, &MenuTool::MenuCommand3),
		FCanExecuteAction());
}


void MenuTool::OnStartupModule()
{
	CommandList = MakeShareable(new FUICommandList);
	MenuToolCommands::Register();
	MapCommands();
	FToolExampleEditor::Get().AddMenuExtension(
		FMenuExtensionDelegate::CreateRaw(this, &MenuTool::MakeMenuEntry),
		FName("Section_1"),
		CommandList);
}

void MenuTool::OnShutdownModule()
{
	MenuToolCommands::Unregister();
}


void MenuTool::MakeMenuEntry(FMenuBuilder &menuBuilder)
{
	/*menuBuilder.AddMenuEntry(MenuToolCommands::Get().MenuCommand1);
	menuBuilder.AddSubMenu(
		FText::FromString("Sub Menu"),
		FText::FromString("This is example sub menu"),
		FNewMenuDelegate::CreateSP(this, &MenuTool::MakeSubMenu)
	);*/

	TSharedRef<SComboButton> SelectAnimSequenceButton =
        SNew(SComboButton)
        .OnGetMenuContent(this, &MenuTool::OnGetAnimMenu)
        .ButtonContent()
        [
            SNew(STextBlock)
            .Text(FText::FromString("Choose Anim Sequence"))
        ];
	
	TSharedRef<SWidget> SelectAnimSequenceWidget =
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SelectAnimSequenceButton
        ];
	SelectAnimSequenceWidgetPtr = SelectAnimSequenceWidget;
	SelectAnimSequenceButtonPtr = SelectAnimSequenceButton;
	
	menuBuilder.AddWidget(SelectAnimSequenceWidget, FText::FromString(""));
	TSharedRef<SWidget> BoneNameWidget =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(50)
			.Text(this, &MenuTool::GetBoneName)
			.OnTextCommitted(this, &MenuTool::OnBoneNameCommitted)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5, 0, 0, 0)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(FText::FromString("Extract Curve"))
			.OnClicked(this, &MenuTool::ExtractBoneCurve)
		];

	menuBuilder.AddWidget(BoneNameWidget, FText::FromString(""));
	
}

void MenuTool::OnAnimAssetSelected(const FAssetData& AssetData)
{
	AnimSequence = Cast<UAnimSequence>(AssetData.GetAsset());
	SelectAnimSequenceButtonPtr->SetIsOpen(false);
}

TSharedRef<SWidget> MenuTool::OnGetAnimMenu()
{
	TArray<const UClass*> ClassFilters;
	ClassFilters.Add(UAnimSequence::StaticClass());
	FAssetData CurrentAssetData = FAssetData();
	AnimSequencePicker = PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
    FAssetData(),
    true,
    ClassFilters,
    TArray<UFactory*>(),
    FOnShouldFilterAsset::CreateLambda([CurrentAssetData](const FAssetData& InAssetData) { return InAssetData == CurrentAssetData; }),
    FOnAssetSelected::CreateSP(this, &MenuTool::OnAnimAssetSelected),
    FSimpleDelegate()
    );
	return AnimSequencePicker.ToSharedRef();
}

FReply MenuTool::OnAnimButtonClicked()
{
	return FReply::Handled();
}

void MenuTool::MakeSubMenu(FMenuBuilder &menuBuilder)
{
	menuBuilder.AddMenuEntry(MenuToolCommands::Get().MenuCommand2);
	menuBuilder.AddMenuEntry(MenuToolCommands::Get().MenuCommand3);	
}

void MenuTool::MenuCommand1()
{
	UE_LOG(LogClass, Log, TEXT("clicked MenuCommand1"));
}

void MenuTool::MenuCommand2()
{
	UE_LOG(LogClass, Log, TEXT("clicked MenuCommand2"));
}

void MenuTool::MenuCommand3()
{
	UE_LOG(LogClass, Log, TEXT("clicked MenuCommand3"));
}


FReply MenuTool::ExtractBoneCurve()
{
	if (FillBoneTransform())
		FillCurveVector();
	return FReply::Handled();
}

FText MenuTool::GetBoneName() const
{
	return FText::FromName(BoneName);
}

void MenuTool::OnBoneNameCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	const FString str = InText.ToString();
	BoneName = FName(*str.TrimStartAndEnd());
}

bool MenuTool::FillBoneTransform() 
{
	BoneTransform.Empty();
	const float DeltaTime = AnimSequence->GetPlayLength() / AnimSequence->GetNumberOfFrames();
	const int BoneIndex = AnimSequence->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE)
		return false;
	for (int i = 0; i < AnimSequence->GetNumberOfFrames(); i++)
	{
		FTransform Out;
		AnimSequence->GetBoneTransform(Out, BoneIndex , i * DeltaTime, true);
		BoneTransform.Add(Out);
	}
	return true;
}

void MenuTool::FillCurveVector() 
{
	FString PackageName = TEXT("/Game/Curve/");
	FString BoneID = BoneName.ToString();
	PackageName += BoneID;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UCurveVector* CurveVectorTrans = NewObject<UCurveVector>(Package, *(BoneID + TEXT("_Trans")), RF_Public | RF_Standalone | RF_MarkAsRootSet);
	UCurveVector* CurveVectorRot = NewObject<UCurveVector>(Package, *(BoneID + TEXT("_Rot")), RF_Public | RF_Standalone | RF_MarkAsRootSet);
	UCurveVector* CurveVectorScale = NewObject<UCurveVector>(Package, *(BoneID + TEXT("_Scale")), RF_Public | RF_Standalone | RF_MarkAsRootSet);

	//Package->AddToRoot();
	//CurveVectorTrans->AddToRoot();
	//CurveVectorRot->AddToRoot();
	//CurveVectorScale->AddToRoot();


	float DeltaTime = AnimSequence->GetPlayLength() / AnimSequence->GetNumberOfFrames();
	FVector Val;
	for (int i = 0; i < BoneTransform.Num(); i++)
	{
		Val = BoneTransform[i].GetLocation();
		CurveVectorTrans->FloatCurves[0].AddKey(i * DeltaTime, Val[0]);
		CurveVectorTrans->FloatCurves[1].AddKey(i * DeltaTime, Val[1]);
		CurveVectorTrans->FloatCurves[2].AddKey(i * DeltaTime, Val[2]);

		Val = BoneTransform[i].GetRotation().Euler();
		CurveVectorRot->FloatCurves[0].AddKey(i * DeltaTime, Val[0]);
		CurveVectorRot->FloatCurves[1].AddKey(i * DeltaTime, Val[1]);
		CurveVectorRot->FloatCurves[2].AddKey(i * DeltaTime, Val[2]);

		Val = BoneTransform[i].GetScale3D();
		CurveVectorScale->FloatCurves[0].AddKey(i * DeltaTime, Val[0]);
		CurveVectorScale->FloatCurves[1].AddKey(i * DeltaTime, Val[1]);
		CurveVectorScale->FloatCurves[2].AddKey(i * DeltaTime, Val[2]);
	}

	FAssetRegistryModule::AssetCreated(CurveVectorTrans);
	FAssetRegistryModule::AssetCreated(CurveVectorRot);
	FAssetRegistryModule::AssetCreated(CurveVectorScale);

	Package->MarkPackageDirty();
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(Package, CurveVectorTrans, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
	UPackage::SavePackage(Package, CurveVectorRot, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
	UPackage::SavePackage(Package, CurveVectorScale, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);

	
}

#undef LOCTEXT_NAMESPACE