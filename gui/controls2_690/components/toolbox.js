.pragma library

// signal helpers

// connect signal only once
function connectOnce(sig, slot) {
    var f = function() { slot.apply(this, arguments); sig.disconnect(f); };
    sig.connect(f);
}

// connect signal while slot return false
function connectWhileFalse(sig, slot) {
    var f = function() { if (slot.apply(this, arguments) === true) sig.disconnect(f); };
    sig.connect(f);
}

// connect signal while slot return true
function connectWhileTrue(sig, slot) {
    var f = function() { if (slot.apply(this, arguments) !== true) sig.disconnect(f); };
    sig.connect(f);
}
