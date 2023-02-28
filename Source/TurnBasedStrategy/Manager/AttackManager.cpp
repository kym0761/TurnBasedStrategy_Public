// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackManager.h"
#include "Kismet/GameplayStatics.h"
#include "UnitCore/UnitAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnitAction/UnitAttackActionComponent.h"
#include "UnitCore/StatComponent.h"
#include "UnitCore/UnitCharacter.h"
#include "GridManager.h"

// Sets default values
AAttackManager::AAttackManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAttackManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAttackManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAttackManager::SetupAttackManager(AActor* Attacker, AActor* Defender)
{
	//Attacker = InAttacker;
	//Defender = InDefender;

	if (!IsValid(Attacker) || !IsValid(Defender))
	{
		return;
	}

	OrderToPlay.Empty();
	OrderToPlay = CalculateAttackOrder(Attacker,Defender);

}

void AAttackManager::StartAttack()
{
	if (OrderToPlay.Num() == 0)
	{
		FinishAttack();
		return;
	}
	FAttackOrder& currentOrder = OrderToPlay[0];

	AActor* currentAttacker = currentOrder.Attacker;
	AActor* currentDefender = currentOrder.Defender;

	if (!IsValid(currentAttacker) || !IsValid(currentDefender))
	{
		FinishAttack();
		return;
	}

	CurrentAttackActionComponent = currentAttacker->FindComponentByClass<UUnitAttackActionComponent>();

	if (!IsValid(CurrentAttackActionComponent))
	{
		//!!��� �ʿ�.
	}


	if (IsValid(currentAttacker))
	{
		FRotator attackerLookRot = UKismetMathLibrary::FindLookAtRotation(currentAttacker->GetActorLocation(), currentDefender->GetActorLocation());
		currentAttacker->SetActorRotation(attackerLookRot);

	}

	if (IsValid(currentDefender))
	{
		FRotator defenderLookRot = UKismetMathLibrary::FindLookAtRotation(currentDefender->GetActorLocation(), currentAttacker->GetActorLocation());
		currentDefender->SetActorRotation(defenderLookRot);
	}

	if (OrderToPlay.Num() > 0)
	{
		PlayAttack();
	}

}

void AAttackManager::PlayAttack()
{
	FAttackOrder& currentOrder = OrderToPlay[0];

	AActor* currentAttacker = currentOrder.Attacker;
	AActor* currentDefender = currentOrder.Defender;

	if (IsValid(currentAttacker) && IsValid(currentDefender))
	{
		USkeletalMeshComponent* attackerMesh = currentAttacker->FindComponentByClass<USkeletalMeshComponent>();
		USkeletalMeshComponent* defenderMesh = currentDefender->FindComponentByClass<USkeletalMeshComponent>();

		if (IsValid(attackerMesh) && IsValid(defenderMesh))
		{
			UUnitAnimInstance* attackerAnim = Cast<UUnitAnimInstance>(attackerMesh->GetAnimInstance());
			UUnitAnimInstance* defenderAnim = Cast<UUnitAnimInstance>(defenderMesh->GetAnimInstance());	

			if (IsValid(attackerAnim) && IsValid(defenderAnim))
			{
				BindOnAttackHit(attackerAnim);
				BindOnAttackEnd(attackerAnim);
				BindOnHit(defenderAnim);
				BindOnHitEnd(defenderAnim);
			
				bAttackerWaiting = false;
				bDefenderWaiting = false;

				//UE_LOG(LogTemp, Warning, TEXT("Play Montage?"));
				attackerAnim->PlayUnitAttackMontage();
			}
		}
	}
}

void AAttackManager::FinishAttack()
{
	if (IsValid(CurrentAttackActionComponent))
	{
		CurrentAttackActionComponent->TakeAction(FGrid(0,0));
	}

	//reset
	bAttackerWaiting = false;
	bDefenderWaiting = false;
	CurrentAttackOrder = FAttackOrder();

}

void AAttackManager::OnAttackHit()
{
	//Attacker�� ������ �¾Ҵٸ� Defender�� hit �ִϸ��̼� �����.

	FAttackOrder& currentOrder = OrderToPlay[0];
	AActor* currentDefender = currentOrder.Defender;

	if (!IsValid(currentOrder.Defender) || !IsValid(currentOrder.Attacker))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(currentOrder.Defender, currentOrder.Damage, nullptr, currentOrder.Attacker, UDamageType::StaticClass());

	if (!IsValid(currentDefender))
	{
		return;
	}

	USkeletalMeshComponent* defenderMesh = currentOrder.Defender->FindComponentByClass<USkeletalMeshComponent>();
	if (!IsValid(defenderMesh))
	{
		return;
	}

	UUnitAnimInstance* defenderAnim = Cast<UUnitAnimInstance>(defenderMesh->GetAnimInstance());
	if (IsValid(defenderAnim))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Play Hit Montage?"));
		defenderAnim->PlayUnitHitMontage();
	}

}

void AAttackManager::OnAttackEnd()
{
	bAttackerWaiting = true;
	UE_LOG(LogTemp, Warning, TEXT("AAttackManager::OnAttackEnd()"));
	TryPlayNextOrder();
}

void AAttackManager::OnHit()
{
	//�Ƹ� ������ ���� ���� ������ Actor�� Spawn?

	//FAttackOrder& currentOrder = OrderToPlay[0];
	//if (!IsValid(currentOrder.Defender) || !IsValid(currentOrder.Attacker))
	//{
	//	return;
	//}

	//UGameplayStatics::ApplyDamage(currentOrder.Defender, currentOrder.Damage, nullptr, currentOrder.Attacker,UDamageType::StaticClass());

}

void AAttackManager::OnHitEnd()
{
	bDefenderWaiting = true;
	UE_LOG(LogTemp, Warning, TEXT("AAttackManager::OnHitEnd()"));
	TryPlayNextOrder();
}

TArray<FAttackOrder> AAttackManager::CalculateAttackOrder(AActor* Attacker, AActor* Defender)
{
	//���� ���� Order�� ���Ƿ� ������ �켱 -> ����� �ݰ���.

	TArray<FAttackOrder> attackOrders;

	if (!IsValid(Attacker) || !IsValid(Defender))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attacker or Defender is Not Valid - AAttackManager::CalculateAttackOrder()"));
		return attackOrders;
	}

	UStatComponent* attackerStatComponent =
		Attacker->FindComponentByClass<UStatComponent>();

	UStatComponent* defenderStatComponent =
		Defender->FindComponentByClass<UStatComponent>();

	if (!IsValid(attackerStatComponent) || !IsValid(defenderStatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("StatComponents are Not Valid - AAttackManager::CalculateAttackOrder()"));

		return attackOrders;
	}

	FAttackOrder attack;
	attack.AttackOrderType = EAttackOrderType::Attack;
	int32 attackerDamage = attackerStatComponent->GetSTR();
	attack.Damage = attackerDamage;
	attack.Attacker = Attacker;
	attack.Defender = Defender;

	FAttackOrder counterAttack;
	counterAttack.AttackOrderType = EAttackOrderType::Defend;
	int32 defenderDamage = defenderStatComponent->GetSTR();
	counterAttack.Damage = defenderDamage;
	counterAttack.Attacker = Defender;
	counterAttack.Defender = Attacker;

	// �׽�Ʈ �뵵�� ��/��/��/�� �������� ������.
	// ���� SPD �� Ȥ��	���� ��ų�� ���� ���� Ƚ���� �޶��� �� ����.
	attackOrders.Add(attack);
	attackOrders.Add(counterAttack);
	attackOrders.Add(attack);
	attackOrders.Add(attack);


	return 	attackOrders;
}

AAttackManager* AAttackManager::GetAttackManager()
{
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport))
	{
		return nullptr;
	}

	UWorld* world = GEngine->GameViewport->GetWorld();
	AAttackManager* attackManager = Cast<AAttackManager>(UGameplayStatics::GetActorOfClass(world, AAttackManager::StaticClass()));

	return attackManager;
}

void AAttackManager::BindOnAttackHit(UUnitAnimInstance* AnimInst)
{
	if (!IsValid(AnimInst))
	{
		return;
	}

	FScriptDelegate toBind;
	toBind.BindUFunction(this, FName("OnAttackHit"));

	AnimInst->BindTo_OnAttackHit(toBind);

}

void AAttackManager::BindOnAttackEnd(UUnitAnimInstance* AnimInst)
{
	if (!IsValid(AnimInst))
	{
		return;
	}

	FScriptDelegate toBind;
	toBind.BindUFunction(this, FName("OnAttackEnd"));

	AnimInst->BindTo_OnAttackEnd(toBind);
}

void AAttackManager::BindOnHit(UUnitAnimInstance* AnimInst)
{
	if (!IsValid(AnimInst))
	{
		return;
	}

	FScriptDelegate toBind;
	toBind.BindUFunction(this, FName("OnHit"));

	AnimInst->BindTo_OnHit(toBind);
}

void AAttackManager::BindOnHitEnd(UUnitAnimInstance* AnimInst)
{
	if (!IsValid(AnimInst))
	{
		return;
	}

	FScriptDelegate toBind;
	toBind.BindUFunction(this, FName("OnHitEnd"));

	AnimInst->BindTo_OnHitEnd(toBind);
}

void AAttackManager::TryPlayNextOrder()
{
	if (bAttackerWaiting && bDefenderWaiting)
	{
		//Remove Finished Order
		if (OrderToPlay.Num() > 0)
		{
			OrderToPlay.RemoveAt(0);
		}

		//������ �׾����� ���� Attack ���� ����.
		if (OrderToPlay.Num() > 0)
		{
			FAttackOrder& currentOrder = OrderToPlay[0];

			if (IsValid(currentOrder.Defender) && IsValid(currentOrder.Attacker))
			{
				UStatComponent* defenderStat = currentOrder.Defender->FindComponentByClass<UStatComponent>();
				UStatComponent* attackerStat = currentOrder.Attacker->FindComponentByClass<UStatComponent>();

				if (IsValid(defenderStat) && IsValid(attackerStat))
				{
					if (defenderStat->GetHP() <= 0.0f || attackerStat->GetHP() <= 0.0f)
					{
						OrderToPlay.Empty();
					}
				}
			}
		}


		if (OrderToPlay.Num() > 0)
		{
			PlayAttack();
		}
		else
		{
			FinishAttack();
		}
	}
}

TArray<FAttackOrder> AAttackManager::GetAttackOrder() const
{
	return OrderToPlay;
}

int32 AAttackManager::CalculateGridValue_ToAttack(AActor* Attacker, AActor* Defender)
{
	//Unit : �� �а� �ʿ�.
    //Grid : Test�� ��ġ.

	if (!IsValid(Attacker) || !IsValid(Defender))
	{
		return -1;
	}

	TArray<FAttackOrder> attackOrders = CalculateAttackOrder(Attacker, Defender);

	UStatComponent* attackerStatComponent =
		Attacker->FindComponentByClass<UStatComponent>();

	UStatComponent* defenderStatComponent =
		Defender->FindComponentByClass<UStatComponent>();

	if (!IsValid(attackerStatComponent) || !IsValid(defenderStatComponent))
	{
		return -1;
	}

	float attackerHP = attackerStatComponent->GetHP();
	float defenderHP = defenderStatComponent->GetHP();

	for (auto attackOrder : attackOrders)
	{
		switch (attackOrder.AttackOrderType)
		{
		case EAttackOrderType::Attack:
			attackerHP -= attackOrder.Damage;
			break;
		case EAttackOrderType::Defend:
			defenderHP -= attackOrder.Damage;
			break;
		}
	}

	//AttackerHP�� �������� ������ ����. DefenderHP�� �������� ������ ����.

	float valueScore = 0.0f;

	float counterAttackValue = 100 * attackerHP / attackerStatComponent->GetMaxHP();

	valueScore += FMath::Clamp<int32>(counterAttackValue, 0.0f, 100.0f);

	float attackValue = 100* (defenderStatComponent->GetMaxHP() - defenderHP) / defenderStatComponent->GetMaxHP();

	valueScore += FMath::Clamp<int32>(attackValue, 0.0f, 100.0f);

	return (int32)valueScore;
}

FAttackOrder::FAttackOrder()
{
	AttackOrderType = EAttackOrderType::Attack;
	Damage = 0;
	Attacker = nullptr;
	Defender = nullptr;
}
