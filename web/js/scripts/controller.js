/**
 * controller.js
 *
 * Place any logic here that doesn't fit easily into view
 */
var magma = magma || {};

magma.controller = (function() {
    // private vars and methods

    // public methods
    return {
        /*** loading ***/
        loading: function() {
            var loadAd = function(adModel) {
                adModel.observeOnce('loaded', function() {
                    if(adModel.hasFact()) {
                        magma.bootstrap.loadingFact(adModel);
                    } else if(adModel.hasAd()) {
                        magma.bootstrap.loadingAd(adModel);
                    }
                });

                var fallback = function() {
                    // log or post error

                    if(adModel.hasFact()) {
                        magma.bootstrap.loadingFact(adModel);
                    } else if(adModel.hasAd()) {
                        magma.bootstrap.loadingAd(adModel);
                    }
                };

                adModel.observeOnce('loadedError', fallback);
                adModel.observeOnce('loadedFailed', fallback);

                adModel.loadAd('loading');
            };

            return {
                loadAd: loadAd
            };
        }
    };
}());
