### Update resource file

    lupdate ../controls2_509/ ../controls2_515/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_en.ts
    lupdate ../controls2_509/ ../controls2_515/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_fr.ts
    lupdate ../controls2_509/ ../controls2_515/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_nl.ts

### Release reviewed translations

    lrelease osmin_en.ts
    lrelease osmin_fr.ts
    lrelease osmin_nl.ts

