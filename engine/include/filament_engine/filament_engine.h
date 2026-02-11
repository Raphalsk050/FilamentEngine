#pragma once

// Master include for the Filament Engine

// Math
#include <filament_engine/math/types.h>

// Core
#include <filament_engine/core/application.h>
#include <filament_engine/core/window.h>
#include <filament_engine/core/input.h>
#include <filament_engine/core/clock.h>
#include <filament_engine/core/event_bus.h>
#include <filament_engine/core/log.h>

// Rendering
#include <filament_engine/rendering/render_context.h>

// ECS
#include <filament_engine/ecs/world.h>
#include <filament_engine/ecs/entity_bridge.h>
#include <filament_engine/ecs/components.h>
#include <filament_engine/ecs/system.h>
#include <filament_engine/ecs/systems/editor_camera_system.h>

// Resources
#include <filament_engine/resources/resource_handle.h>
#include <filament_engine/resources/resource_manager.h>
#include <filament_engine/resources/mesh.h>
#include <filament_engine/resources/material.h>
