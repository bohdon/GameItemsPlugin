# Game Items Plugin

An Unreal plugin with classes and tools for creating gameplay items, inventories and equipment.

## Features

- `UGameItemDef`, a blueprintable game item definition, extensible via modular `UGameItemFragment` objects which allow
  compositing item functionality, e.g. UI data, what equipment to grant, what stats to define, etc. (just like Lyra)
- `UObject` based `UGameItem` instances which support replication, per-item change events, a 'stack count' and
  open-ended set of gameplay tag numerical stats.
- A highly versatile `UGameItemContainer`, which can represent anything from inventories to single equip slots
  and quick-use item bars.
- Container rules which define behavior like which items are allowed in a container, stack and
  total item count limits, automatic slotting rules, and complex parent-child container relationships.
- Game equipment that can be added and removed from actors to grant abilities and visual effects, applied automatically
  based on customizable world conditions such as whether the item is equipped to a certain container.
- Drop tables that use instanced structs to specify drop contents and probabilities and support contextual gameplay tag
  stats to further affect drop results.
- "Econ Value" concept that makes it easy to balance economies by basing item exchange quantities off a generalized
  global value.
- Move, split, stack, swap and transfer items amongst containers with ease.

# Game Items UI Plugin

An accompanying plugin for building game item and container UIs.

## Features

- Uses the MVVM plugin to provide easy access to item and container properties.
- View models for game item containers, slots and items.
- View model resolvers for automatically locating player or other game item containers by gameplay tag.
- Utils for easily performing common UI actions like moving, stacking, swapping, and equipping items.

# Gameplay Ability Items

An additional plugin for building game equipment that grants abilities using the Gameplay Ability System.

Requires the [ExtendedGameplayAbilities](https://github.com/bohdon/ExtendedGameplayAbilitiesPlugin) plugin which is used
to define ability sets (groups of related abilities, effects, and attribute sets).

## Features

- `UAbilityEquipmentDef` which specifies an array of `UExtendedAbilitySets` to grant when equipped.
- `UAbilityEquipment` class that handles granting the abilities, optionally using game item tag stats to determine the
  level of the granted abilities or effects.
