// Which title-bar implementation owns the screen: our compositor plugin (primary), or the layer-shell bars
// as a fallback. The plugin is ABI-locked to one Hyprland build; its .abi receipt starts with that build's commit.

pub const PLUGIN_NAME: &str = "kwinish-bars";

#[derive(Debug, PartialEq)]
pub enum Mode {
    /// our plugin is loaded: it draws the bars, the layer-shell daemon stays idle
    Plugin,
    /// another title-bar plugin (hyprbars) is loaded: drawing a second bar would double up
    OtherBars(String),
    /// the plugin is installed and built for this Hyprland but not loaded yet
    TryLoad,
    /// the plugin is installed but built for another Hyprland: emergency mode, with a notice
    Emergency { built: String, running: String },
    /// no plugin installed: plain fallback, no notice
    Fallback,
}

pub struct Facts<'a> {
    pub loaded: &'a [String],
    pub installed: bool,
    pub abi_commit: Option<&'a str>, // first "_"-separated field of the .abi receipt
    pub running_commit: &'a str,
}

pub fn decide(f: &Facts) -> Mode {
    if f.loaded.iter().any(|n| n == PLUGIN_NAME) {
        return Mode::Plugin;
    }
    if let Some(other) = f.loaded.iter().find(|n| n.as_str() == "hyprbars") {
        return Mode::OtherBars(other.clone());
    }
    if !f.installed {
        return Mode::Fallback;
    }
    match f.abi_commit {
        Some(c) if c == f.running_commit => Mode::TryLoad,
        Some(c) => Mode::Emergency { built: c.chars().take(7).collect(), running: f.running_commit.chars().take(7).collect() },
        // an installed plugin with no receipt cannot be checked: treat as stale rather than risk loading it
        None => Mode::Emergency { built: "unknown".into(), running: f.running_commit.chars().take(7).collect() },
    }
}

pub fn emergency_text(built: &str, running: &str) -> String {
    format!(
        "HyperVi: emergency mode. The title-bar plugin was built for Hyprland {built} but {running} is running, so it was not loaded. \
Fallback bars are active and may sit above other windows or be clipped. Rebuild the kwinish-bars plugin for a consistent look."
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    fn facts<'a>(loaded: &'a [String], installed: bool, abi: Option<&'a str>) -> Facts<'a> {
        Facts { loaded, installed, abi_commit: abi, running_commit: "efb5099aaaa" }
    }

    #[test]
    fn modes() {
        let none: Vec<String> = vec![];
        assert_eq!(decide(&facts(&none, false, None)), Mode::Fallback);
        assert_eq!(decide(&facts(&none, true, Some("efb5099aaaa"))), Mode::TryLoad);
        assert_eq!(decide(&facts(&none, true, Some("1234567bbbb"))), Mode::Emergency { built: "1234567".into(), running: "efb5099".into() });
        assert!(matches!(decide(&facts(&none, true, None)), Mode::Emergency { .. }));
        let ours = vec![PLUGIN_NAME.to_string()];
        assert_eq!(decide(&facts(&ours, true, Some("old"))), Mode::Plugin); // loaded wins: Hyprland already vetted it
        let other = vec!["hyprbars".to_string()];
        assert_eq!(decide(&facts(&other, false, None)), Mode::OtherBars("hyprbars".into()));
    }
}
