profile(general, privacy, public).

profile(silent, privacy, private).

profile(meeting, privacy, private).

profile(outdoor, privacy, public).

current_profile(X) :-
    fact_exists('com.nokia.policy.current_profile', [value], [X]).

/*
current_profile(X) :-
    related(current_profile, [X]).
*/
