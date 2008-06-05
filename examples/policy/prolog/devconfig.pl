/*
 * policy_groups
 */
policy_group(sink  , cscall).
policy_group(source, cscall).
policy_group(sink  , ringtone).
policy_group(sink  , ipcall).
policy_group(source, ipcall).
policy_group(sink  , player).
policy_group(sink  , fmradio).
policy_group(sink  , othermedia).
policy_group(source, othermedia).

/*
 * Here is a bunch of exception for audio routing
 */
invalid_audio_device_choice(Group, SinkOrSource, _, Device) :-
    (not(Group=ringtone), not(audio:privacy_override(_)) ->
         findall(P, audio:available_accessory(SinkOrSource, P), AccessoryList),
         not(AccessoryList==[]),
         not(member(Device, AccessoryList))
    ).

invalid_audio_device_choice(_, sink, public, _) :-
    not(audio:privacy_override(public)),
    audio:active_policy_group(cscall).

invalid_audio_device_choice(_, sink, _, earpiece) :-
    audio:active_policy_group(ringtone).

invalid_audio_device_choice(othermedia, sink, _, earpiece).
invalid_audio_device_choice(player    , sink, _, earpiece).

invalid_audio_device_choice(Group, sink, _, ihfandheadset) :-
     not(Group=ringtone).

/*
invalid_audio_device_choice(_, sink, _, earpiece) :-
    audio:active_policy_group(player).
*/

invalid_audio_device_choice(_, source, _, headmike).

/*
 * Volume Control
 */
/*           Activated   ToBeLimited         Level */
/*           ------------------------------------- */
volume_limit(ringtone  , sink  , player    , 0    ).
volume_limit(ringtone  , sink  , fmradio   , 0    ).
volume_limit(ringtone  , sink  , othermedia, 0    ).
volume_limit(cscall    , sink  , player    , 10   ).
volume_limit(cscall    , sink  , fmradio   , 10   ).
volume_limit(cscall    , sink  , othermedia, 10   ).
volume_limit(ipcall    , sink  , player    , 10   ).
volume_limit(ipcall    , sink  , fmradio   , 10   ).
volume_limit(ipcall    , sink  , othermedia, 10   ).
volume_limit(player    , sink  , fmradio   , 0    ).
volume_limit(player    , sink  , othermedia, 0    ).
volume_limit(fmradio   , sink  , player    , 0    ).
volume_limit(fmradio   , sink  , othermedia, 0    ).
volume_limit(othermedia, sink  , player    , 100  ).
volume_limit(othermedia, sink  , fmradio   , 100  ).

% We need at least one 'volume_limit_exception' predicate.
% So if there is no other you have to add
%   volume_limit_exception(_, _, _) :- fail. 

volume_limit_exception(fmradio, sink, Limit) :-
    (audio:current_route(sink, earpiece),
     not(audio:active_policy_group(cscall)),
     not(audio:active_policy_group(ringtone)) *->
         Limit=0
    ).

/*
 * pausing
 */

% We need at least one 'enforce_pausing' predicate.
% So if there is no other you have to add
%   enforce_pausing(_, _) :- fail. 

enforce_pausing(othermedia, earpiece) :-
    not(audio:active_policy_group(cscall)),
    not(audio:active_policy_group(ringtone)).

enforce_pausing(player, earpiece) :-
    not(audio:active_policy_group(cscall)),
    not(audio:active_policy_group(ringtone)).


/*
 * Play Request handling
 */
/*
audio_play_preconditions(PolicyGroup, MediaType, PreCondList) :-
*/
audio_play_preconditions(_, _, PreCondList) :-
    PreCondList=[[cpu_frequency_request, 300], [cpu_load, lt, 50]].

reject_audio_play_request(_, _) :-
    fail.
