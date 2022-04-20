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

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh")); // Создать скелет и назвать его (TEXT("ItemMesh")).
	SetRootComponent(ItemMesh); // Сделать ItemMesh к корневым.

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox")); // Создание объекта столкновение, при помощи него будут появляться виджеты с пояснениями о свойствах предмета.
	CollisionBox->SetupAttachment(ItemMesh); // Прикрепление его к корневому компоненту(RootComponent)
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // Игнорирование всех коллизий.
	CollisionBox->SetCollisionResponseToChannel( // Блокирует, если будет попытка линии трассировки.
		ECollisionChannel::ECC_Visibility,
		ECollisionResponse::ECR_Block); // Блокировать канал видимости коллизии.

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget")); // Подключение виджет-компонента к сетке BP_BaseWeapon
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere")); // Получение площади сферы.
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	PickupWidget->SetVisibility(false); // Спрятать PickupWidget.

	// Установить наложение для AreaSphere. Позволяет получить адресс функции и хранить это.
	AreaSphere->OnComponentBeginOverlap.AddDynamic( // Связывает UObject экземпляр и UObject метод обращается к этому многоадресному делегату.
		this, // Объект, который использует утилиту.
		&AItem::OnSphereOverlap); // Имя функции, для привязки OnComponentBeginOverlap передаст всю эту информацию связанной с ним функции. Вызывается при спорикосновении с площадью сферы.

	AreaSphere->OnComponentEndOverlap.AddDynamic(
		this, 
		&AItem::OnSphereEndOverlap); // Вызывается, когда прекращается контактирования с площадью сферы.
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/*Переменная на сфере области, унаследована от UPrimitiveComponent,
которая позволяет взять функцию и привязать её, после чего каждый раз,
когда кто-то пересекается с компонентом сферы, функция будет вызываться.*/
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) // Пересекается с площадью сферы и вызывает событие.
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor); // Переделывает OtherActor в AShooterCharacter.
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1); // Прибавляет к счётчику единицу.
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) // Пересекается с площадью сферы и вызывает событие.
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor); // Переделывает OtherActor в AShooterCharacter.
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1); // Уменьшает на единицу счётчик.
		}
	}
}

