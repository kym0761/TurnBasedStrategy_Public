// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitActionListWidget.h"
#include "../UnitCharacter.h"
#include "../UnitAction/UnitActionComponent.h"
#include "ActionSelectButtonWidget.h"
#include "Components/VerticalBox.h"

void UUnitActionListWidget::InitUnitActionsWidget(AUnitCharacter* SelectedCharacter)
{
	if (!IsValid(SelectedCharacter))
	{
		return;
	}

	TArray<UActorComponent*> unitActions;

	SelectedCharacter->GetComponents(UUnitActionComponent::StaticClass(), unitActions);

	for (UActorComponent* unitAction : unitActions)
	{
		UUnitActionComponent* unitAction_Cast=
		Cast<UUnitActionComponent>(unitAction);

		if (!IsValid(unitAction_Cast))
		{
			continue;
		}

		//�� �׼��� �̹� �Ͽ� ����� ��������
		if (!unitAction_Cast->IsCanDoActionThisTurn())
		{
			continue;
		}

		//�� �׼��� ���� ��ġ���� ��밡���� ĭ�� �����ϴ���
		if (!unitAction_Cast->IsCurrentlyAvailableAction())
		{
			continue;
		}

		UActionSelectButtonWidget* buttonWidget = CreateWidget<UActionSelectButtonWidget>(GetWorld(), ChooseActionButtonWidgetClass);
		
		if (!IsValid(buttonWidget))
		{
			continue;
		}

		VerticalBox_ActionList->AddChild(buttonWidget);
		buttonWidget->InitActionSelectButton(unitAction_Cast);
		buttonWidget->OnButtonClickedCompleted.AddDynamic(this, &UUnitActionListWidget::OnButtonClickedCompletedFunc);

		//buttonWidget->OnButtonClickedCompleted.RemoveDynamic(this, &UUnitActionListWidget::OnButtonClickedCompletedFunc);
	}
}

void UUnitActionListWidget::OnButtonClickedCompletedFunc()
{
	RemoveFromParent();
}
