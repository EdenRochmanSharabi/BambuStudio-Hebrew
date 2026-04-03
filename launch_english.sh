#!/bin/bash
# Restore BambuStudio to English (and restore original Spanish .mo)

APP="/Applications/BambuStudio.app"
CONF="$HOME/Library/Application Support/BambuStudio/BambuStudio.conf"
ES_MO="$APP/Contents/Resources/i18n/es/BambuStudio.mo"
BACKUP="$APP/Contents/Resources/i18n/es/BambuStudio.mo.original"

# Restore original Spanish .mo if backup exists
if [ -f "$BACKUP" ]; then
    cp "$BACKUP" "$ES_MO"
fi

# Set language to English
sed -i '' 's/"language": "[^"]*"/"language": "en_GB"/' "$CONF"

echo "Launching BambuStudio in English..."
open "$APP"
