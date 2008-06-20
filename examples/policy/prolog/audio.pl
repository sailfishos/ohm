:- module(audio,
	  [set_routes/1, set_volume_limits/1, set_corks/1,
	   group_playback_request/4, update_playback_entries/4
	   available_accessory/2, privacy_override/1, active_policy_group/1,
	   current_route/2, privacy/1, connected/1]).

/*
 * Generic rules
 */
head([Elem|_], Elem).


/*
 * Audio Devices
 */
available_device(SinkOrSource, Privacy, Device) :-
    audio_device(SinkOrSource, Privacy, Device),
    connected(Device).
    
available_accessory(SinkOrSource, Device) :-
    accessory(Device),
    connected(Device),
    audio_device(SinkOrSource, _, Device).


/*
 * Privacy
 */
privacy_map(private, private).
privacy_map(public , private).
privacy_map(public , public).

privacy(Privacy) :-
    (privacy_override(P) *->
         Privacy=P
     ;
         current_profile(Profile),
         profile(Profile, privacy, P),
         privacy_map(P, Privacy)
    ).


/*
 * routing 
 */
device_candidate(SinkOrSource, Device) :-
    privacy(Privacy),
    available_device(SinkOrSource, Privacy, Device),
    policy_group(SinkOrSource, Group),
    active_policy_group(Group),
    not(invalid_audio_device_choice(Group, SinkOrSource, Privacy, Device)).

device_candidate_list(SinkOrSource, DeviceList) :-
    findall(S, device_candidate(SinkOrSource, S), DeviceList).

route_to_device(SinkOrSource, Device) :-
    device_candidate_list(SinkOrSource, DeviceList),
    (current_profile(Profile),
     profile(Profile, device, PreferredDevice),
     member(PreferredDevice, DeviceList) *->
         Device = PreferredDevice
     ;
         head(DeviceList, Device)
    ).

route_action(Action) :-
    audio_device_type(SinkOrSource),
    route_to_device(SinkOrSource, Device),
    Action=[audio_route, [type, SinkOrSource], [device, Device]].

set_routes(ActionList) :-
    findall(A, route_action(A), ActionList).




/*
 * Volume limit
 */
group_volume_limit_candidate(GroupToLimit, SinkOrSource, Limit) :-
    (policy_group(SinkOrSource, Group),
     (active_policy_group(Group) *->
          volume_limit(Group, SinkOrSource, GroupToLimit, Limit)
      ;
          Limit=100
     )
    );
    volume_limit_exception(GroupToLimit, SinkOrSource, Limit).

group_volume_limit(Group, SinkOrSource, Limit) :-
    findall(L, group_volume_limit_candidate(Group, SinkOrSource, L),
	    LimitList),
    sort(LimitList, SortedLimitList),
    head(SortedLimitList, Limit).

volume_limit_action(Action) :-
    policy_group(sink, Group),
    group_volume_limit(Group, sink, NewLimit),
    Action=[volume_limit, [group, Group], [limit, NewLimit]].

set_volume_limits(ActionList) :-
    findall(A, volume_limit_action(A), ActionList).


/*
 * Cork
 */
cork_action(Action) :-
    policy_group(sink, Group),
    (route_to_device(sink,_) *->
         Action=[cork_stream, [group, Group], [cork, uncorked]]
     ;
         Action=[cork_stream, [group, Group], [cork, corked]]
    ).

set_corks(ActionList) :-
    findall(A, cork_action(A), ActionList).


/*
 * Group Playback Requests
 */
policy_group_state_list_elem(GrantedGroup, GrantedState, Elem) :-
    policy_group_state(Group, State),
    (Group = GrantedGroup *->
         Elem = [active_policy_group, [group, Group], [state, GrantedState]]
     ;
         Elem = [active_policy_group, [group, Group], [state, State]]
    ).

policy_group_state_list(GrantedGroup, GrantedState, List) :-
    findall(E, policy_group_state_list_elem(GrantedGroup, GrantedState, E),
	    List).

group_playback(PolicyGroup, play, MediaType, GroupStateList) :-
    not(reject_group_play_request(PolicyGroup, MediaType)),
    policy_group_state_list(PolicyGroup, '1', GroupStateList).
	    
group_playback(PolicyGroup, stop, _, GroupStateList) :-
    policy_group_state_list(PolicyGroup, '0', GroupStateList).
	    

group_playback_request(PolicyGroup, State, MediaType, GroupStateList) :-
    group_playback(PolicyGroup, State, MediaType, GroupStateList).


/*
 *
 */
playback_entry_elem(DBusID,Object, Pid,Stream, Group,
		    ReqState,State,SetState, Elem) :-
    Elem = [playback,
	    [dbusid, DBusID], [object, Object],
	    [pid, Pid], [stream, Stream],
	    [group, Group],
	    [reqstate, ReqState], [state, State], [setstate, SetState]
           ].

playback_entry(ReqPid, ReqStream, stop, Elem) :-
    current_playback(DBusID,Object,Pid,Stream, Group, ReqState,State,SetState),
    (Pid = ReqPid, Stream = ReqStream *->
         playback_entry_elem(DBusID,Object, Pid,Stream, Group,
			     ReqState,State,stop, Elem)
     ;
         playback_entry_elem(DBusID,Object, Pid,Stream, Group,
			     ReqState,State,SetState, Elem)
    ).

playback_entry(ReqPid, ReqStream, play, Elem) :- 
    current_playback(_,_, ReqPid,ReqStream, ReqGroup, _,_,_),
    current_playback(DBusID,Object,Pid,Stream, Group, ReqState,State,SetState),
    (Group = ReqGroup *->
         (Pid = ReqPid, Stream = ReqStream *->
	      playback_entry_elem(DBusID,Object, Pid,Stream, Group,
				  ReqState,State,play, Elem)
	  ;
	      playback_entry_elem(DBusID,Object, Pid,Stream, Group,
				  ReqState,State,stop, Elem)
	 )
     ;
         playback_entry_elem(DBusID,Object, Pid,Stream, Group,
			     ReqState,State,SetState, Elem)
    ).

update_playback_entries(Pid, Stream, State, PlaybackList) :-
    findall(E, playback_entry(Pid, Stream, State, E), PlaybackList).

/*
 * system state
 */

privacy_override(X) :-
    fact_exists('com.nokia.policy.privacy_override',
		[value], [X]),
    not(X=default).

connected(X) :-
    fact_exists('com.nokia.policy.accessories',
		[device, state],
		[X, '1']).

policy_group_state(Group, State) :-
    fact_exists('com.nokia.policy.audio_active_policy_group',
		[group, state],
		[Group, State]).

active_policy_group(X) :-
    fact_exists('com.nokia.policy.audio_active_policy_group',
		[group, state],
		[X, '1']).

current_route(SinkOrSource, Where) :-
    fact_exists('com.nokia.policy.audio_route',
		[type, device], [SinkOrSource, Where]).

current_volume_limit(PolicyGroup, Limit) :-
    fact_exists('com.nokia.policy.volume_limit',
		[group, limit],
		[PolicyGroup, Limit]).

current_cork(PolicyGroup, Corked) :-
    fact_exists('com.nokia.policy.audio_cork',
		[group, cork],
		[PolicyGroup, Corked]).

current_playback(DBusID,Object, Pid,Stream, PolicyGroup,
		 ReqState, State, SetState) :-
    fact_exists('com.nokia.policy.playback',
		[dbusid,object, pid,stream, group,
		 reqstate, state, setstate],
		[DBusID,Object, Pid,Stream, PolicyGroup,
		 ReqState, State, SetState]).
