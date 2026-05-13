#include "pch.h"
#include "TitleBookActor.h"

void TitleBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);
    SetInitPageState(BookPageState::SecondPage);
}

void TitleBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
}

void TitleBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

