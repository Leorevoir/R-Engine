template<typename T>
void r::ecs::Commands::add_component(Entity e, T comp) const
{
    if (_scene) {
        _scene->add_component<T>(e, std::move(comp));
    }
}
