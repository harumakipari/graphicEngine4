#include "pch.h"
#include "ResultBookActor.h"

void ResultBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);
    SetInitPageState(BookPageState::SecondPage);
}

void ResultBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
}

void ResultBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

