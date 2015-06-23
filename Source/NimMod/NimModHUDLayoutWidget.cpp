// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModHUD.h"
#include "NimModHUDLayoutWidget.h"

ANimModHUD *UNimModHUDLayoutWidget::GetOwningHUD()
{
	return OwningHUD;
}
