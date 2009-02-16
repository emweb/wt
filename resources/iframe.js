/*
 * LICENSE: see orbited_LICENSE.txt
 */
function extract_xss_domain(old_domain) {
    domain_pieces = old_domain.split('.')
    if (domain_pieces.length == 4) {
        var is_ip = true;
        for (var i = 0; i < 4; ++i) {
            var n = Number(domain_pieces[i])
            if (isNaN(n)) {
                is_ip = false;
            }
        }
        if (is_ip) {
            return old_domain;
        }
    }
    return domain_pieces.slice(-2, domain_pieces.length).join(".")
}

function reload() {
}

window.onError = null;
document.domain = extract_xss_domain(document.domain);
parent.Orbited.attach_iframe(this);
// FIXME: Define this in orbited.js
p = function() {}
