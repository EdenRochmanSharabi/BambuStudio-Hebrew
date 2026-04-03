#!/bin/bash
# Launch BambuStudio in Hebrew
# Swaps the Spanish .mo with Hebrew and sets language to es (Spanish)
# since the unmodified binary doesn't have Hebrew in its language list

APP="/Applications/BambuStudio.app"
CONF="$HOME/Library/Application Support/BambuStudio/BambuStudio.conf"
ES_MO="$APP/Contents/Resources/i18n/es/BambuStudio.mo"
HE_MO="$APP/Contents/Resources/i18n/he/BambuStudio.mo"
BACKUP="$APP/Contents/Resources/i18n/es/BambuStudio.mo.original"

# Backup original Spanish if not already backed up
if [ ! -f "$BACKUP" ]; then
    cp "$ES_MO" "$BACKUP"
fi

# Swap in Hebrew .mo
cp "$HE_MO" "$ES_MO"

# Set language to Spanish (loads our Hebrew .mo)
sed -i '' 's/"language": "[^"]*"/"language": "es"/' "$CONF"

echo "Launching BambuStudio in Hebrew..."
open "$APP"
