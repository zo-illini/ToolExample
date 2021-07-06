
//#include "AssetRegistryModule.h"
//#include "ScopedTransaction.h"
//#include "SDockTab.h"
//#include "SDockableTab.h"
//#include "SDockTabStack.h"
//#include "SlateApplication.h"
#include "ToolExampleEditor/ToolExampleEditor.h"
#include "TabToolPanel.h"
#include "TabTool.h"

#include "PropertyCustomizationHelpers.h"

void TabTool::OnStartupModule()
{
	FExampleTabToolBase::OnStartupModule();
	FToolExampleEditor::Get().AddMenuExtension(FMenuExtensionDelegate::CreateRaw(this, &TabTool::MakeMenuEntry), FName("Section_2"));
}

void TabTool::OnShutdownModule()
{
	FExampleTabToolBase::OnShutdownModule();
}

void TabTool::Initialize()
{
	TabName = "TabTool";
	TabDisplayName = FText::FromString("Open CurveTool Tab");
	ToolTipText = FText::FromString("CurveTool Window");

	TransScale = FText::FromString("1");
	RotScale = FText::FromString("1");
	AnimSequence = nullptr;
}

TSharedRef<SDockTab> TabTool::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	
	// ComboButton + AssetPicker for anim sequence
	
	TSharedRef<SComboButton> SelectAnimSequenceButton =
        SNew(SComboButton)
        .OnGetMenuContent(this, &TabTool::OnGetAnimMenu)
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

	// Cache for later adjustment
	SelectAnimSequenceWidgetPtr = SelectAnimSequenceWidget;
	SelectAnimSequenceButtonPtr = SelectAnimSequenceButton;

	TSharedRef<SWidget> ScalingFactorWidget =
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
			
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString("Translation Scale"))
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(5, 0, 0, 0)
            .VAlign(VAlign_Center)
            [
                SNew(SEditableTextBox)
                .MinDesiredWidth(50)
                .Text(this, &TabTool::GetTransScale)
                .OnTextCommitted(this, &TabTool::OnTransScaleCommitted)
            ]
        ]
    .Padding(0, 0, 0, 5)
        + SVerticalBox::Slot()
        [
				
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromString("Rotation Scale"))
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(5, 0, 0, 0)
            .VAlign(VAlign_Center)
            [
                SNew(SEditableTextBox)
                .MinDesiredWidth(50)
                .Text(this, &TabTool::GetRotScale)
                .OnTextCommitted(this, &TabTool::OnRotScaleCommitted)
            ]
        ];
	
	// Enter and Display bone name, Button for extracting curve
	
	TSharedRef<SWidget> BoneNameWidget =
    	SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString("Enter Bone Name"))
        ]
    	+ SHorizontalBox::Slot()
    	.AutoWidth()
		.Padding(5, 0, 0, 0)
    	.VAlign(VAlign_Center)
    	[
    	    SNew(SEditableTextBox)
    	    .MinDesiredWidth(50)
    	    .Text(this, &TabTool::GetBoneName)
    	    .OnTextCommitted(this, &TabTool::OnBoneNameCommitted)
    	];

	TSharedRef<SWidget> ExtractCurveButton =
		SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(FText::FromString("Extract Curve"))
			.OnClicked(this, &TabTool::ExtractBoneCurve)
		];
		
	
	// Thumbnail for Anim Sequence
	
	AnimThumbnailPoolPtr = MakeShared<FAssetThumbnailPool>(10, true);
	AnimThumbnailPtr = MakeShareable(new FAssetThumbnail(FAssetData(), 128, 128, AnimThumbnailPoolPtr));
	TSharedRef<SWidget> AnimThumbnailBox =
    	SNew(SHorizontalBox)
    	+ SHorizontalBox::Slot()
    	.MaxWidth(128)
    	[
    	    AnimThumbnailPtr->MakeThumbnailWidget()
    	];


	// Finally, create and return the tab
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
      	.TabRole(ETabRole::NomadTab)
      	[
      	    SNew(TabToolPanel)
      	    .Tool(SharedThis(this))
      	]
      	.ShouldAutosize(true)
      	.Content()
      	[
          	SNew(SVerticalBox)
          	+ SVerticalBox::Slot()
          	.MaxHeight(32)
          	[
          	    SelectAnimSequenceWidget
          	]
          	+ SVerticalBox::Slot()
          	.MaxHeight(128)
          	[
          	    AnimThumbnailBox
          	]
          	+ SVerticalBox::Slot()
          	.MaxHeight(32)
          	[
          	    BoneNameWidget
          	]
          	+SVerticalBox::Slot()
            .MaxHeight(64)
            [
                ScalingFactorWidget
            ]
          	+SVerticalBox::Slot()
          	.MaxHeight(32)
          	[
          		ExtractCurveButton
          	]
      ];
		return SpawnedTab;
}


void TabTool::OnAnimAssetSelected(const FAssetData& AssetData)
{
	AnimSequence = Cast<UAnimSequence>(AssetData.GetAsset());
	SelectAnimSequenceButtonPtr->SetIsOpen(false);
	TSharedRef<SWidget> NewButton =
		SNew(SComboButton)
            .OnGetMenuContent(this, &TabTool::OnGetAnimMenu)
            .ButtonContent()
            [
                SNew(STextBlock)
                .Text(FText::FromName(AssetData.AssetName))
            ];
	SelectAnimSequenceButtonPtr->SetContent(NewButton);
	AnimThumbnailPtr->SetAsset(AssetData);
	AnimThumbnailPtr->RefreshThumbnail();
	
	//StaticCastSharedPtr<FAssetThumbnail, SWidget, ESPMode::Fast>(AnimThumbnailPtr)->SetAsset(AssetData);
}

TSharedRef<SWidget> TabTool::OnGetAnimMenu()
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
    FOnAssetSelected::CreateSP(this, &TabTool::OnAnimAssetSelected),
    FSimpleDelegate()
    );
	return AnimSequencePicker.ToSharedRef();
}

FReply TabTool::OnAnimButtonClicked()
{
	return FReply::Handled();
}

FReply TabTool::ExtractBoneCurve()
{
	FillCurveVector();
	SaveCurve();
	return FReply::Handled();
}

FText TabTool::GetBoneName() const
{
	return FText::FromName(BoneName);
}


FText TabTool::GetTransScale() const
{
	return TransScale;
}

FText TabTool::GetRotScale() const
{
	return RotScale;
}

void TabTool::OnBoneNameCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	const FString str = InText.ToString();
	BoneName = FName(*str.TrimStartAndEnd());
}

void TabTool::OnTransScaleCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	const FString str = InText.ToString();
	TransScale = InText;
}

void TabTool::OnRotScaleCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	const FString str = InText.ToString();
	RotScale = InText;
}

/*bool TabTool::FillBoneTransform() 
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
}*/


void TabTool::FillCurveVector() 
{
	if(!AnimSequence)
		return;
	
	TransCurve.Reset();
	RotCurve.Reset();
	ScaleCurve.Reset();
	
	const float TScale = FCString::Atof(*TransScale.ToString());
	const float RScale = FCString::Atof(*RotScale.ToString());
	const float DeltaTime = AnimSequence->GetPlayLength() / AnimSequence->GetNumberOfFrames();
	const int BoneIndex = AnimSequence->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);

	if (BoneIndex != INDEX_NONE)
	{
		for (int i = 0; i < AnimSequence->GetNumberOfFrames(); i++)
		{
			FTransform Out;
			AnimSequence->GetBoneTransform(Out, BoneIndex , i * DeltaTime, true);
			TransCurve.Add(Out.GetLocation() * TScale);
			RotCurve.Add(Out.GetRotation().Euler() * RScale);
			ScaleCurve.Add(Out.GetScale3D());
		}
	}
}


void TabTool::SaveCurve()
{

	if (TransCurve.Num() == 0 ||
		RotCurve.Num() == 0 ||
		ScaleCurve.Num() == 0)
		return;
	
	FString PackageName = TEXT("/Game/Curve/");
	const FString AnimName = AnimSequence->GetName();
	const FString BoneID = BoneName.ToString();
	const FString CurveName = AnimName + "_" + BoneID;
	PackageName += AnimName + "/" + BoneID;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	UCurveVector* CurveVectorTrans = NewObject<UCurveVector>(Package, *(CurveName + TEXT("_Trans")), RF_Public | RF_Standalone | RF_MarkAsRootSet);
	UCurveVector* CurveVectorRot = NewObject<UCurveVector>(Package, *(CurveName + TEXT("_Rot")), RF_Public | RF_Standalone | RF_MarkAsRootSet);
	UCurveVector* CurveVectorScale = NewObject<UCurveVector>(Package, *(CurveName + TEXT("_Scale")), RF_Public | RF_Standalone | RF_MarkAsRootSet);


	float DeltaTime = AnimSequence->GetPlayLength() / AnimSequence->GetNumberOfFrames();
	for (int i = 0; i < AnimSequence->GetNumberOfFrames(); i++)
	{
		FVector Val = TransCurve[i];
		CurveVectorTrans->FloatCurves[0].AddKey(i * DeltaTime, Val[0]);
		CurveVectorTrans->FloatCurves[1].AddKey(i * DeltaTime, Val[1]);
		CurveVectorTrans->FloatCurves[2].AddKey(i * DeltaTime, Val[2]);

		Val = RotCurve[i];
		CurveVectorRot->FloatCurves[0].AddKey(i * DeltaTime, Val[0]);
		CurveVectorRot->FloatCurves[1].AddKey(i * DeltaTime, Val[1]);
		CurveVectorRot->FloatCurves[2].AddKey(i * DeltaTime, Val[2]);

		Val = ScaleCurve[i];
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