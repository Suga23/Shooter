#include "ShooterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h" // KismetMathLibrary.

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)	// Если каждый кадр, выделенный на анимацию null, то функция внутри попробует сделать снова.
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		// Получение боковой(горизонтальной) скорости персонажа от скорости(velocity)
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;

		/* Значение скорости(Speed) не принимает Z компонент скорости
		в учётную запись. Если персонаж, во время прыжка, летит вверх 
		или падает вниз, это не оказывает воздействия на переменную Speed. */
		Speed = Velocity.Size(); // Величина(magnitude) скорости вектора.

		// Если персонаж падает - true, если не падает - false.
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling(); // Если персонаж в воздухе.

		// Если персонаж ускоряется?
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAcceleration = true; // Если вектор больше нуля.
		}
		else
		{
			bIsAcceleration = false; // Если вектор меньше нуля.
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation(); // Возвращает вращение соответствующее направлению прицеливания.
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX( // // Получить вращение от X.
			ShooterCharacter->GetVelocity()); // Соответствие вращения скорости.
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(
			MovementRotation,
			AimRotation).Yaw;

		// Даже если скорость 0, нет необходимости обновлять это, нужно лишь хранить значение с последнего раза, когда скорость не была равна нулю.
		if (ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			// Как только персонаж прекращает движение, сохраняется последняя компенсация движения Yaw.
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		bAiming = ShooterCharacter->GetAiming();

		// Распечатывает сообщение на экране показывающее на сколько персонаж повернулся относительно нуля на координатной плоскости.
		// FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f), AimRotation.Yaw);

		// FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f), MovementRotation.Yaw);
		// FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementRotation.Yaw);

		/*if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, // Ключ.
				0.f, // Время отображения(распечатывается каждый кадр).
				FColor::White, // Цвет.
				OffsetMessage); // Сообщение.
		}*/
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}