### Update resource file

    lupdate ../controls2_690/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_en.ts
    lupdate ../controls2_690/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_fr.ts
    lupdate ../controls2_690/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_nl.ts
    lupdate ../controls2_690/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_de.ts
    lupdate ../controls2_690/ ../../backend/libosmscout/libosmscout-client-qt -ts osmin_es.ts

### Release reviewed translations

    lrelease osmin_en.ts
    lrelease osmin_fr.ts
    lrelease osmin_nl.ts
    lrelease osmin_de.ts
    lrelease osmin_es.ts

