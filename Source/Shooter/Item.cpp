#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh")); // ������� ������ � ������� ��� (TEXT("ItemMesh")).
	SetRootComponent(ItemMesh); // ������� ItemMesh � ��������.

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox")); // �������� ������� ������������, ��� ������ ���� ����� ���������� ������� � ����������� � ��������� ��������.
	CollisionBox->SetupAttachment(ItemMesh); // ������������ ��� � ��������� ����������(RootComponent)
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // ������������� ���� ��������.
	CollisionBox->SetCollisionResponseToChannel( // ���������, ���� ����� ������� ����� �����������.
		ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block); // ����������� ����� ��������� ��������.

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget")); // ����������� ������-���������� � ����� BP_BaseWeapon
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere")); // ��������� ������� �����.
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	PickupWidget->SetVisibility(false); // �������� PickupWidget.

	// ���������� ��������� ��� AreaSphere. ��������� �������� ������ ������� � ������� ���.
	AreaSphere->OnComponentBeginOverlap.AddDynamic( // ��������� UObject ��������� � UObject ����� ���������� � ����� �������������� ��������.
		this, // ������, ������� ���������� �������.
		&AItem::OnSphereOverlap); // ��� �������, ��� �������� OnComponentBeginOverlap �������� ��� ��� ���������� ��������� � ��� �������. ���������� ��� ��������������� � �������� �����.

	AreaSphere->OnComponentEndOverlap.AddDynamic(
		this, 
		&AItem::OnSphereEndOverlap); // ����������, ����� ������������ ��������������� � �������� �����.
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/*���������� �� ����� �������, ������������ �� UPrimitiveComponent,
������� ��������� ����� ������� � ��������� �, ����� ���� ������ ���,
����� ���-�� ������������ � ����������� �����, ������� ����� ����������.*/
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) // ������������ � �������� ����� � �������� �������.
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor); // ������������ OtherActor � AShooterCharacter.
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1); // ���������� � �������� �������.
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) // ������������ � �������� ����� � �������� �������.
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor); // ������������ OtherActor � AShooterCharacter.
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1); // ��������� �� ������� �������.
		}
	}
}

