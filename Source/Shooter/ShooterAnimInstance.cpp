#include "ShooterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h" // KismetMathLibrary.

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)	// ���� ������ ����, ���������� �� �������� null, �� ������� ������ ��������� ������� �����.
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		// ��������� �������(��������������) �������� ��������� �� ��������(velocity)
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;

		/* �������� ��������(Speed) �� ��������� Z ��������� ��������
		� ������� ������. ���� ��������, �� ����� ������, ����� ����� 
		��� ������ ����, ��� �� ��������� ����������� �� ���������� Speed. */
		Speed = Velocity.Size(); // ��������(magnitude) �������� �������.

		// ���� �������� ������ - true, ���� �� ������ - false.
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling(); // ���� �������� � �������.

		// ���� �������� ����������?
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAcceleration = true; // ���� ������ ������ ����.
		}
		else
		{
			bIsAcceleration = false; // ���� ������ ������ ����.
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation(); // ���������� �������� ��������������� ����������� ������������.
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX( // // �������� �������� �� X.
			ShooterCharacter->GetVelocity()); // ������������ �������� ��������.
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(
			MovementRotation,
			AimRotation).Yaw;

		// ���� ���� �������� 0, ��� ������������� ��������� ���, ����� ���� ������� �������� � ���������� ����, ����� �������� �� ���� ����� ����.
		if (ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			// ��� ������ �������� ���������� ��������, ����������� ��������� ����������� �������� Yaw.
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		bAiming = ShooterCharacter->GetAiming();

		// ������������� ��������� �� ������ ������������ �� ������� �������� ���������� ������������ ���� �� ������������ ���������.
		// FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f), AimRotation.Yaw);

		// FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f), MovementRotation.Yaw);
		// FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f"), MovementRotation.Yaw);

		/*if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, // ����.
				0.f, // ����� �����������(��������������� ������ ����).
				FColor::White, // ����.
				OffsetMessage); // ���������.
		}*/
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}