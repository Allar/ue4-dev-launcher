#include "../AllarBuilderClientApp.h"
#include "ISourceCodeAccessModule.h"
#include "SClientLauncher.h"
#include "SCookProgress.h"
#include "SCookAndDeploy.h"

#define LOCTEXT_NAMESPACE "CookAndDeploy"

void SCookAndDeploy::Construct(const FArguments& InArgs, TSharedRef<FAllarBuilderClient> InClient, const TSharedRef<ISlateStyle>& InStyle)
{
	Client = InClient;

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 8.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CookAndDeploy", "Cook"))
			.TextStyle(InStyle, TEXT("Section.Label"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(5.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.HAlign(HAlign_Left)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.Padding(8.0f)
				[
					SNew(SBox)
					.WidthOverride(650)
					[
						SNew(SGridPanel)
						.FillColumn(0, 0.5f)
						.FillColumn(1, 1)
						.FillColumn(2, 1)
						.FillColumn(3, 1)
						.FillColumn(4, 1)
						// Platforms label
						+ SGridPanel::Slot(0,0)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Platforms", "Platforms:"))
							.TextStyle(InStyle, TEXT("Section.Text"))
						]
						// Windows
						+ SGridPanel::Slot(1,0)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(WindowsCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("Windows", "Windows"))
							.RightAlignCheckBox(true)
						]
						// WindowsServer
						+ SGridPanel::Slot(2,0)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(WindowsServerCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("WindowsServer", "WinServer"))
							.RightAlignCheckBox(true)
						]
						// Linux
						+ SGridPanel::Slot(3,0)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(LinuxCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("Linux", "Linux"))
							.RightAlignCheckBox(true)
						]
						// LinuxServer
						+ SGridPanel::Slot(4,0)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(LinuxServerCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("LinuxServer", "LinuxServer"))
							.RightAlignCheckBox(true)
						]
						// Row spacer		
						+ SGridPanel::Slot(0, 1)
						[
							SNew(SSpacer)
							.Size(FVector2D(8, 8))
						]
						// Options label			
						+ SGridPanel::Slot(0,2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Options", "Options:"))
							.TextStyle(InStyle, TEXT("Section.Text"))
						]
						// Use Pak files?
						+ SGridPanel::Slot(1,2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(PakCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("UsePakFiles", "Use .pak"))
							.RightAlignCheckBox(true)
						]
						// Compress?
						+ SGridPanel::Slot(2,2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(CompressCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("Compress", "Compress"))
							.RightAlignCheckBox(true)
						]
						// Iterate?
						+ SGridPanel::Slot(3,2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(IterateCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("Iterate", "Iterate"))
							.RightAlignCheckBox(true)
						]
						// Strip Version?
						+ SGridPanel::Slot(4,2)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(StripVersionCheckboxOption, SCheckboxOption, InStyle)
							.LabelText(LOCTEXT("StripVersion", "StripVersion"))
							.RightAlignCheckBox(true)
						]
						// Row spacer		
						+ SGridPanel::Slot(0, 3)
						[
							SNew(SSpacer)
							.Size(FVector2D(8, 8))
						]
						// Begin Cook Button
						+ SGridPanel::Slot(0,4)
						.ColumnSpan(5)
						[
							SNew(SButton)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.IsEnabled(this, &SCookAndDeploy::IsCookingEnabled)
							.OnClicked(this, &SCookAndDeploy::StartCook)
							.Content()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("StartCook", "Start Cook"))
							]
						]
					]
				]
			]
			// Cook Progress
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SAssignNew(CookProgress, SCookProgress, InClient, InStyle)
			]
		]
	];
}

bool SCookAndDeploy::IsCookingEnabled() const
{
	if (CookProgress.IsValid())
	{
		return CookProgress->IsReadyForNewJob();
	}
	return false;
}

FReply SCookAndDeploy::StartCook()
{
	if (CookProgress.IsValid() && CookProgress->IsReadyForNewJob())
	{
		CookProgress->ClearTasks();

		FString CookArgs = TEXT("BuildCookRun");
		CookArgs += TEXT(" -project=\"") + Client->GetProjectPath() + TEXT("\"");
		CookArgs += TEXT(" -noP4 -nocompileeditor -utf8output -cook -map= -stage -package -clientconfig=Development -serverconfig=Development");

		if (FRocketSupport::IsRocket())
		{
			CookArgs += TEXT(" -rocket -nocompile");
		}

		// See if project has code that needs to be built
		FString ProjectPath = FPaths::GetPath(Client->GetProjectPath());
		TArray<FString> OutProjectCodeFilenames;
		IFileManager::Get().FindFilesRecursive(OutProjectCodeFilenames, *(ProjectPath / TEXT("Source")), TEXT("*.h"), true, false, false);
		IFileManager::Get().FindFilesRecursive(OutProjectCodeFilenames, *(ProjectPath / TEXT("Source")), TEXT("*.cpp"), true, false, false);
		ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>("SourceCodeAccess");
		if (OutProjectCodeFilenames.Num() > 0 && SourceCodeAccessModule.GetAccessor().CanAccessSourceCode())
		{
			CookArgs += TEXT(" -build");
		}	

		// Strip versions from content?
		if (StripVersionCheckboxOption->CheckBox->IsChecked())
		{
			CookArgs += TEXT(" -unversionedcookedcontent");
		}

		// Use .pak files?
		if (StripVersionCheckboxOption->CheckBox->IsChecked())
		{
			CookArgs += TEXT(" -pak");
		}

		// Iterative cook?
		if (IterateCheckboxOption->CheckBox->IsChecked())
		{
			CookArgs += TEXT(" -iterativecooking");
		}

		// Compress build?
		if (CompressCheckboxOption->CheckBox->IsChecked())
		{
			CookArgs += TEXT(" -compressed");
		}

		// Build Windows Task
		if (WindowsCheckboxOption->CheckBox->IsChecked() || WindowsServerCheckboxOption->CheckBox->IsChecked())
		{
			FString PlatformString;
			FString TaskDesc;
			if (WindowsCheckboxOption->CheckBox->IsChecked())
			{
				PlatformString += TEXT(" -platform=Win64");
				TaskDesc += TEXT("Win64");
			}

			if (WindowsServerCheckboxOption->CheckBox->IsChecked())
			{
				PlatformString += TEXT(" -server -serverplatform=Win64");
				TaskDesc += TaskDesc.Len() > 0 ? TEXT(" and ") : TEXT("");
				TaskDesc += ("Win64 Server");
			}

			CookProgress->NewTask(Client->GetEngineBatchFilesPath() / TEXT("RunUAT.bat"), CookArgs + PlatformString, Client->GetEngineBatchFilesPath(), TEXT("WindowsCook"), TEXT("Cooking ") + TaskDesc);
		}

		// Build Linux Task
		if (LinuxCheckboxOption->CheckBox->IsChecked() || LinuxServerCheckboxOption->CheckBox->IsChecked())
		{
			FString PlatformString;
			FString TaskDesc;
			if (LinuxCheckboxOption->CheckBox->IsChecked())
			{
				PlatformString += TEXT(" -platform=Linux");
				TaskDesc += TEXT("Linux");
			}

			if (LinuxServerCheckboxOption->CheckBox->IsChecked())
			{
				PlatformString += TEXT(" -server -serverplatform=Linux");
				TaskDesc += TaskDesc.Len() > 0 ? TEXT(" and ") : TEXT("");
				TaskDesc += ("Linux Server");
			}

			CookProgress->NewTask(Client->GetEngineBatchFilesPath() / TEXT("RunUAT.bat"), CookArgs + PlatformString, Client->GetEngineBatchFilesPath(), TEXT("LinuxCook"), TEXT("Cooking ") + TaskDesc);
		}

		CookProgress->ExecuteTasks();
	}
	
	return FReply::Handled();
}