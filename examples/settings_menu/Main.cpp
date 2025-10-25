#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Plugins/UiPlugin.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <R-Engine/UI/Button.hpp>
#include <R-Engine/UI/Components.hpp>
#include <R-Engine/UI/Events.hpp>
#include <R-Engine/UI/Text.hpp>
#include <R-Engine/UI/Theme.hpp>

#include <string>

enum class SettingsTab {
	None,
	Video,
	Audio,
	Controls,
	Accessibility,
};
struct TabButton {
	SettingsTab tab = SettingsTab::None;
};

struct PanelsResource {
	r::ecs::Entity video = r::ecs::NULL_ENTITY;
	r::ecs::Entity audio = r::ecs::NULL_ENTITY;
	r::ecs::Entity controls = r::ecs::NULL_ENTITY;
	r::ecs::Entity accessibility = r::ecs::NULL_ENTITY;
};

struct SettingsState {
	SettingsTab current = SettingsTab::Video;
	r::ecs::Entity label_entity = r::ecs::NULL_ENTITY;
};

// Tag component to uniquely identify the right-side label entity
struct RightLabelTag {};

static void setup_theme(r::ecs::ResMut<r::UiTheme> theme, r::ecs::ResMut<r::UiPluginConfig> cfg)
{
	// Désactiver l'overlay debug pour cet exemple
	cfg.ptr->show_debug_overlay = false;

	// Couleurs pour la sidebar et les boutons
	theme.ptr->button.bg_normal = r::Color{18, 18, 20, 255};       // très sombre
	theme.ptr->button.bg_hover = r::Color{60, 60, 65, 220};
	theme.ptr->button.bg_pressed = r::Color{90, 90, 95, 220};
	theme.ptr->button.bg_disabled = r::Color{40, 40, 40, 255};

	theme.ptr->button.border_normal = r::Color{120, 190, 200, 255};
	theme.ptr->button.border_hover = r::Color{120, 190, 200, 255};
	theme.ptr->button.border_pressed = r::Color{120, 190, 200, 255};
	theme.ptr->button.border_disabled = r::Color{80, 80, 80, 255};

	theme.ptr->button.border_thickness = 2.f;
	theme.ptr->button.text = r::Color{200, 230, 235, 255};

	// Fond par défaut un gris très foncé
	theme.ptr->panel_bg = r::Color{8, 8, 10, 255};
}

static void build_settings_menu(r::ecs::Commands &cmds, r::ecs::Res<r::WindowPluginConfig> win)
{
	(void) win;

	// Root: ligne avec sidebar à gauche et contenu à droite
	cmds.spawn(r::UiNode{},
			r::Style{
				.width_pct = 100.f,
				.height_pct = 100.f,
				.background = r::Color{0,0,0,0},
				.margin = 0.f,
				.padding = 0.f,
				.direction = r::LayoutDirection::Row,
				.gap = 0.f,
			},
			r::ComputedLayout{},
			r::Visibility::Visible)
		.with_children([&](r::ecs::ChildBuilder &parent) {
			// Sidebar gauche: largeur fixe, prend toute la hauteur
			parent.spawn(r::UiNode{},
				r::Style{
					.width = 220.f,
					.height_pct = 100.f,
					.background = r::Color{14, 14, 16, 255},
					.margin = 0.f,
					.padding = 20.f,
					.direction = r::LayoutDirection::Column,
					.justify = r::JustifyContent::SpaceBetween,
					.align = r::AlignItems::Center,
					.gap = 8.f,
				},
				r::ComputedLayout{},
				r::Visibility::Visible)
			.with_children([&](r::ecs::ChildBuilder &side) {
				// On crée un conteneur supérieur pour coller le premier bouton en haut
				side.spawn(r::UiNode{},
					r::Style{
						.width_pct = 100.f,
						.background = r::Color{0,0,0,0},
						.margin = 0.f,
						.padding = 0.f,
						.direction = r::LayoutDirection::Column,
						.justify = r::JustifyContent::Start,
						.align = r::AlignItems::Center,
						.gap = 8.f,
					},
					r::ComputedLayout{},
					r::Visibility::Visible)
				.with_children([&](r::ecs::ChildBuilder &col) {
					// Boutons (alignés haut -> bas via SpaceBetween sur le parent de la sidebar)
					const float btn_w_pct = 100.f;
					const float btn_h = 80.f;

					col.spawn(r::UiNode{}, r::UiButton{}, TabButton{SettingsTab::Video},
						r::Style{
							.width = 180.f,
							.height = btn_h,
							.background = r::Color{0,0,0,0},
							.direction = r::LayoutDirection::Column,
							.justify = r::JustifyContent::Center,
							.align = r::AlignItems::Center,
						},
						r::UiText{.content = std::string("Video"), .font_size = 20},
						r::ComputedLayout{}, r::Visibility::Visible);

					col.spawn(r::UiNode{}, r::UiButton{}, TabButton{SettingsTab::Audio},
						r::Style{
							.width = 180.f,
							.height = btn_h,
							.background = r::Color{0,0,0,0},
							.direction = r::LayoutDirection::Column,
							.justify = r::JustifyContent::Center,
							.align = r::AlignItems::Center,
						},
						r::UiText{.content = std::string("Audio"), .font_size = 20},
						r::ComputedLayout{}, r::Visibility::Visible);

					col.spawn(r::UiNode{}, r::UiButton{}, TabButton{SettingsTab::Controls},
						r::Style{
							.width = 180.f,
							.height = btn_h,
							.background = r::Color{0,0,0,0},
							.direction = r::LayoutDirection::Column,
							.justify = r::JustifyContent::Center,
							.align = r::AlignItems::Center,
						},
						r::UiText{.content = std::string("Controls"), .font_size = 20},
						r::ComputedLayout{}, r::Visibility::Visible);

					col.spawn(r::UiNode{}, r::UiButton{}, TabButton{SettingsTab::Accessibility},
						r::Style{
							.width = 180.f,
							.height = btn_h,
							.background = r::Color{0,0,0,0},
							.direction = r::LayoutDirection::Column,
							.justify = r::JustifyContent::Center,
							.align = r::AlignItems::Center,
						},
						r::UiText{.content = std::string("Accessibility"), .font_size = 18},
						r::ComputedLayout{}, r::Visibility::Visible);
				});

				// Petit spacer flexible si besoin (SpaceBetween du parent gère l'espacement global)
				side.spawn(r::UiNode{},
					r::Style{
						.width_pct = 100.f,
						.height_pct = 100.f,
						.background = r::Color{0,0,0,0},
					},
					r::ComputedLayout{}, r::Visibility::Visible);

			// Right content panel (take remaining space) with a title label showing current section
			parent.spawn(r::UiNode{},
				 r::Style{
					.width_pct = 100.f,
					.height_pct = 100.f,
					.background = r::Color{0,0,0,0},
					.padding = 20.f,
					.direction = r::LayoutDirection::Column,
					.justify = r::JustifyContent::Start,
					.align = r::AlignItems::Start,
					.gap = 12.f,
				},
				 r::ComputedLayout{}, r::Visibility::Visible)
			.with_children([&](r::ecs::ChildBuilder &content) {
			 // Label at the top that will display current tab (Video/Audio/...)
			 auto label_ent = content.spawn(r::UiNode{},
					 r::Style{
						.height = 56.f,
						.width_pct = 100.f,
						.background = r::Color{0,0,0,0},
						.direction = r::LayoutDirection::Column,
						.justify = r::JustifyContent::Center,
						.align = r::AlignItems::Start,
					},
				 // attach the RightLabelTag so we can find this specific UiText later
				 RightLabelTag{},
				 r::UiText{.content = std::string("Video"), .font_size = 28, .color = r::Color{200,230,235,255}},
					 r::ComputedLayout{}, r::Visibility::Visible).id();

				// Placeholder content area
				content.spawn(r::UiNode{},
					 r::Style{
						.width_pct = 100.f,
						.height_pct = 100.f,
						.background = r::Color{12,12,14,255},
						.padding = 12.f,
					},
					 r::ComputedLayout{}, r::Visibility::Visible);

				// Register the label entity in a resource so other systems can update it
				cmds.insert_resource(SettingsState{SettingsTab::Video, label_ent});
				// Log the label entity id for debugging
				r::Logger::info(std::string("Right label entity id: ") + std::to_string(static_cast<uint64_t>(label_ent)));
			});

			});
		});
}

static void settings_click_system(r::ecs::EventReader<r::UiClick> click_reader,
    r::ecs::Query<r::ecs::Ref<TabButton>> buttons,
    r::ecs::ResMut<SettingsState> state,
    r::ecs::Query<r::ecs::Mut<r::UiText>> all_texts,
    r::ecs::Query<r::ecs::Mut<r::UiText>, r::ecs::With<RightLabelTag>> label_texts,
    r::ecs::Commands &cmds)
{
    static bool first_log = true; // Ensure we log all UiText entities only once
    if (first_log) {
        for (auto dit = all_texts.begin(); dit != all_texts.end(); ++dit) {
            auto [dt] = *dit;
            std::string content = dt.ptr ? dt.ptr->content : std::string("<no-text>");
            r::Logger::info(std::string("UiText entity: ") + std::to_string(static_cast<uint64_t>(dit.entity())) + std::string(" -> ") + content);
        }
        first_log = false;
    }

    // Log the current label entity only if it changes
    static r::ecs::Entity last_label_entity = r::ecs::NULL_ENTITY;
    if (state.ptr->label_entity != last_label_entity) {
        r::Logger::info("Current label entity: " + std::to_string(static_cast<uint64_t>(state.ptr->label_entity)));
        last_label_entity = state.ptr->label_entity;
    }

    for (const auto &click : click_reader) {
        const auto clicked = click.entity;
        if (clicked == r::ecs::NULL_ENTITY) continue;

        r::Logger::info("Click detected on entity: " + std::to_string(static_cast<uint64_t>(clicked)));

        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            auto [btn] = *it;
            if (!btn.ptr) continue;
            if (static_cast<r::ecs::Entity>(it.entity()) == clicked) {
                std::string new_label;
                switch (btn.ptr->tab) {
                    case SettingsTab::Video:
                        new_label = "Video";
                        state.ptr->current = SettingsTab::Video;
                        break;
                    case SettingsTab::Audio:
                        new_label = "Audio";
                        state.ptr->current = SettingsTab::Audio;
                        break;
                    case SettingsTab::Controls:
                        new_label = "Controls";
                        state.ptr->current = SettingsTab::Controls;
                        break;
                    case SettingsTab::Accessibility:
                        new_label = "Accessibility";
                        state.ptr->current = SettingsTab::Accessibility;
                        break;
                    default:
                        break;
                }

                // Try to directly mutate the right-side label UiText (tagged with RightLabelTag)
                bool mutated = false;
                if (state.ptr->label_entity != r::ecs::NULL_ENTITY && !new_label.empty()) {
                    for (auto it = label_texts.begin(); it != label_texts.end(); ++it) {
                        auto [t, _tag] = *it;
                        if (t.ptr) {
                            t.ptr->content = new_label;
                            t.ptr->font_size = 28;
                            t.ptr->color = r::Color{200,230,235,255};
                            mutated = true;
                            r::Logger::info(std::string("Label component content now: ") + t.ptr->content);
                            break;
                        }
                    }

                    // Fallback: if the tagged component wasn't present in the query, ensure the entity has the UiText next frame
                    if (!mutated) {
                        r::Logger::warn("Label entity not found in query. Inserting new UiText component.");
                        cmds.entity(state.ptr->label_entity).insert(r::UiText{.content = new_label, .font_size = 28, .color = r::Color{200,230,235,255}});
                    }
                } else {
                    r::Logger::error("Label entity is NULL_ENTITY or new_label is empty.");
                }

                // Log which tab was clicked
                r::Logger::info(std::string("Sidebar: ") + new_label + std::string(" clicked"));
                break;
            }
        }
    }
}

int main()
{
	r::Application{}
		.add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
			.size = {1280, 720},
			.title = "R-Type - Settings",
			.cursor = r::WindowCursorState::Visible,
		}}))
		.add_plugins(r::UiPlugin{})
		.add_systems<setup_theme, build_settings_menu>(r::Schedule::STARTUP)
		.add_systems<settings_click_system>(r::Schedule::UPDATE)
		.after<r::ui::pointer_system>()
		.run();
}

